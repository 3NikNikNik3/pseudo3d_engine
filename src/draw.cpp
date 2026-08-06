#include "draw.hpp"

#include <stdlib.h>
#include <fstream>
#include <iostream>

#include <GL/glew.h>

#include "engine_structs.hpp"

#define vertex_line_arr sf::Vertex arr[4] = { \
			{ { (float)x, (float)y } }, \
			{ { x + 1.0f, (float)y } }, \
			{ { (float)x, (float)y + size } }, \
			{ { x + 1.0f, (float)y + size } } };


namespace pseudo3d_engine {
	namespace draw {
		// Image
		bool Image::load(const char *path) {
			for (size = 1; path[size]; ++size);

			this->path = new char[size + 1];
			for (int i = 0; i <= size; ++i)
				this->path[i] = path[i];

			sf::Image image;
			if (!image.loadFromFile(path))
				return false;
			return texture.loadFromImage(image);
		}

		void Image::unload() {
			delete[] path;
		}

		int Image::get_x() {
			return texture.getSize().x;
		}

		int Image::get_y() {
			return texture.getSize().y;
		}

		// shader's functions
		bool load_shader(GLuint type, const char *path, GLuint &ans) {
			std::ifstream file(path);
			if (!file.is_open()) {
				std::cerr << "\033[31mError load shader \"" << path << "\"\033[39m: no file" << std::endl;
				return false;
			}

			file.seekg(0, std::ios::end);
			int len = file.tellg();
			file.seekg(0, std::ios::beg);

			char *src = new char[len + 1];
			file.read(src, len);
			src[len] = '\0';

			file.close();

			ans = glCreateShader(type);
			glShaderSource(ans, 1, &src, nullptr);
			glCompileShader(ans);

			delete[] src;

			GLint ret;
			glGetShaderiv(ans, GL_COMPILE_STATUS, &ret);
			if (!ret) {
				char text[512];
				glGetShaderInfoLog(ans, 512, nullptr, text);
				std::cerr << "\033[31mError compile shader \"" << path << "\"\033[39m: " << text << std::endl;

				glDeleteShader(ans);
				return false;
			}

			return true;
		}

		bool check_link_prog(GLuint prog) {
			GLint ret;
			glGetProgramiv(prog, GL_LINK_STATUS, &ret);
			if (!ret) {
				char text[512];
				glGetProgramInfoLog(prog, 512, nullptr, text);
				std::cerr << "\033[31mError link program\033[39m: " << text << std::endl;
				return false;
			}
			return true;
		}

		// global var. for opengl
		static GLuint prog_color = 0;
		static GLuint vbo = 0;

		//! delete ../
		bool init() {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glGenBuffers(1, &vbo);

			GLuint sh_vec;
			if (!load_shader(GL_VERTEX_SHADER, "../shaders/vertex.vert", sh_vec))
				return false;

			// prog_color
			GLuint sh_color;
			if (!load_shader(GL_FRAGMENT_SHADER, "../shaders/color.frag", sh_color)) {
				glDeleteShader(sh_vec);
				return false;
			}

			prog_color = glCreateProgram();
			glAttachShader(prog_color, sh_vec);
			glAttachShader(prog_color, sh_color);

			glLinkProgram(prog_color);
			if (!check_link_prog(prog_color)) {
				glDeleteShader(sh_vec);
				glDeleteShader(sh_color);
				return false;
			}

			glDeleteShader(sh_color);

			glDeleteShader(sh_vec);

			return true;
		}

		// buffer
		struct buffer_draw {
			float s, t;
			float from_x, from_y;
			float a_x, a_y;
			std::uint16_t id_wall;
			uchar id_world;
			uchar time: 1;
			uchar none: 1;
		};

		static buffer_draw *buff = nullptr;
		static unsigned int size = 0;

		static uchar *use = nullptr, *time = nullptr;

		void init_buff(unsigned int size_new) {
			if (size_new != size) {
				std::free(buff);
				std::free(use);
				std::free(time);

				buff = (buffer_draw*)std::calloc(size_new * DEPTH, sizeof(buffer_draw));
				use = (uchar*)std::calloc((DEPTH >> 3) + 1, sizeof(uchar));
				time = (uchar*)std::calloc((DEPTH >> 3) + 1, sizeof(uchar));

				if (buff == nullptr || use == nullptr || time == nullptr)
					throw "little memory";

				size = size_new;
			} else {
				for (int i = DEPTH >> 3; i > -1; --i)
					use[i] = 0;
			}
		}

                void deinit() {
			std::free(buff);
			std::free(use);
			std::free(time);

			glDeleteProgram(prog_color);
			glDeleteBuffers(1, &vbo);
		}

                void add(unsigned int x, uchar depth, float s, float t, float from_x, float from_y, float a_x, float a_y, std::uint16_t id_wall, uchar id_world) { 
			use[depth >> 3] |= 1 << (depth & 0x7);

			buffer_draw &now = buff[depth * size + x];
			now.s = s;
			now.t = t;
			now.from_x = from_x;
			now.from_y = from_y;
			now.a_x = a_x;
			now.a_y = a_y;
			now.id_world = id_world;
			now.id_wall = id_wall;
			now.none = 0;

			now.time = (~(time[depth >> 3] >> (depth & 0x7))) & 1;
		}

		void add_none(unsigned int x, uchar depth, float from_x, float from_y, float a_x, float a_y, uchar id_world) {
			use[depth >> 3] |= 1 << (depth & 0x7);

			buffer_draw &now = buff[depth * size + x];
			now.from_x = from_x;
			now.from_y = from_y;
			now.a_x = a_x;
			now.a_y = a_y;
			now.id_world = id_world;
			now.none = 1;

			now.time = (~(time[depth >> 3] >> (depth & 0x7))) & 1;
		}

		// for compact writing
		struct vec2f {
			GLfloat x, y;
		};

		inline void draw_wall(Window &win, Universe &uni, int i, int j, int x, int y, int size_x, int size_y, const buffer_draw &buff_now, vec2f *arr_point, int len) {
			const Wall &wall = uni.worlds[buff_now.id_world].walls[buff_now.id_wall];
			if (wall.draw_type) {
				for (int k = 0; k < len; ++k) {
					const buffer_draw &buff_just_now = buff[i * size + j + k];
					const float x_now = x + j + k,
						    y_now = y + size_y / 2.0 * (1 - 1 / buff_just_now.s);

					vec2f *arr = arr_point + size_x * 18 * i + size_x * 6 + (j + k) * 6;

					arr[0] = { x_now, y_now };
					arr[1] = { x_now + 1, y_now };
					arr[2] = { x_now + 1, y_now + size_y / buff_just_now.s };
					arr[3] = { x_now + 1, y_now + size_y / buff_just_now.s };
					arr[4] = { x_now, y_now };
					arr[5] = { x_now, y_now + size_y / buff_just_now.s };

					/*if (wall.draw_type == 1) {
						if (wall.type == 3) {
							arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
								{ uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].r,
								  uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].g,
								  uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].b,
								  uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].alpha };
						} else {
							arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
								{ wall.r, wall.g, wall.b, wall.alpha };
						}
					} else if (wall.draw_type == 2) {
						Image *img;
						if (wall.type == 3)
							img = &uni.images[uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].id_texture];
						else
							img = &uni.images[wall.id_texture];
						const float pos_x = img->get_x() * buff[i * size + j + k].t, pos_y = img->get_y();

						arr[k * 6].texCoords = { pos_x - 0.5f, 0 };
						arr[k * 6 + 1].texCoords = { pos_x + 0.5f, 0 };
						arr[k * 6 + 2].texCoords = { pos_x + 0.5f, pos_y };
						arr[k * 6 + 3].texCoords = { pos_x + 0.5f, pos_y };
						arr[k * 6 + 4].texCoords = { pos_x - 0.5f, 0 };
						arr[k * 6 + 5].texCoords = { pos_x - 0.5f, pos_y };
					}*/
				}

				if (wall.draw_type == 1) {
					GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
					GLint color = glGetUniformLocation(prog_color, "color_");

					glUseProgram(prog_color);

					glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
					if (wall.type == 3)
						glUniform4f(color, uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].r,
								   uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].g,
								   uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].b,
								   uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].alpha);
					else
						glUniform4f(color, wall.r, wall.g, wall.b, wall.alpha);

					glDrawArrays(GL_TRIANGLES, size_x * 18 * i + size_x * 6 + j * 6, len * 6);
				} /*else if (wall.draw_type == 2) {
					Image *img;
					if (wall.type == 3)
						img = &uni.images[uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].id_texture];
					else
						img = &uni.images[wall.id_texture];

					win.window->draw(arr, len * 6, sf::PrimitiveType::Triangles, &img->texture);
				}*/
			}
		}

                void draw(Window &win, Universe &uni, int x, int y, int size_x, int size_y) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, DEPTH * size_x * 3 * 6 * 2 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

			vec2f *arr_point = (vec2f*)glMapBufferRange(GL_ARRAY_BUFFER, 0, DEPTH * size_x * 3 * 6 * 2 * sizeof(GLfloat), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			if (!arr_point) {
				while (GLint er = glGetError())
					std::cerr << "\033[31mOpenGL error\033[39m: " << er << std::endl;
				return;
			}

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);

			for (int i = DEPTH - 1; i > -1; --i) {
				if ((use[i >> 3] >> (i & 0x7)) & 1) {
					for (int j = 0; j < size;) {
						while (j < size && buff[i * size + j].time == ((time[i >> 3] >> (i & 0x7)) & 1)) {
							buff[i * size + j].time ^= 1;
							++j;
						}

						if (j == size)
							break;

						int len = 1;
						while (j + len < size && buff[i * size + j + len].time != ((time[i >> 3] >> (i & 0x7)) & 1) && buff[i * size + j + len].id_world == buff[i * size + j].id_world && buff[i * size + j + len].id_wall == buff[i * size + j].id_wall && buff[i * size + j + len].none == buff[i * size + j].none)
							++len;

						if (buff[i * size + j].none) {
							// draw infinity

							// up-place
							Place *place = &uni.worlds[buff[i * size + j].id_world].up;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = size_y / 2.0f;

								vec2f *arr = arr_point + size_x * 18 * i + (j + k) * 6;

								arr[0] = { pos_x, (float)y };
								arr[1] = { pos_x + 1, (float)y };
								arr[2] = { pos_x + 1, y + pos_y };
								arr[3] = { pos_x + 1, y + pos_y };
								arr[4] = { pos_x, (float)y };
								arr[5] = { pos_x, y + pos_y };

								/*if (place->draw_type) { // sky
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								} else { // place
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								}*/
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawArrays(GL_TRIANGLES, size_x * 18 * i + j * 6, len * 6);
							}

							// down-place
							place = &uni.worlds[buff[i * size + j].id_world].down;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = size_y / 2;

								vec2f *arr = arr_point + size_x * 18 * i + size_x * 12 + (j + k) * 6;

								arr[0] = { pos_x, y + pos_y };
								arr[1] = { pos_x, (float)y + size_y };
								arr[2] = { pos_x + 1, (float)y + size_y };
								arr[3] = { pos_x + 1, (float)y + size_y };
								arr[4] = { pos_x + 1, y + pos_y };
								arr[5] = { pos_x, y + pos_y };

								/*if (place->draw_type) { // sky?!
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								} else { // place
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								}*/
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawArrays(GL_TRIANGLES, size_x * 18 * i + size_x * 12 + j * 6, len * 6);
							}
						} else {
							// draw normal
							const buffer_draw &buff_now = buff[i * size + j];

							// walls
							draw_wall(win, uni, i, j, x, y, size_x, size_y, buff_now, arr_point, len);

							// up-place
							Place *place = &uni.worlds[buff[i * size + j].id_world].up;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = size_y / 2.0f * (1 - 1 / just_now_buff.s);

								vec2f *arr = arr_point + size_x * 18 * i + (j + k) * 6;

								arr[0] = { pos_x, (float)y };
								arr[1] = { pos_x, y + pos_y };
								arr[2] = { pos_x + 1, y + pos_y };
								arr[3] = { pos_x + 1, y + pos_y };
								arr[4] = { pos_x + 1, (float)y };
								arr[5] = { pos_x, (float)y };

								/*if (place->draw_type) { // sky
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								} else { // place
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								}*/
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawArrays(GL_TRIANGLES, size_x * 18 * i + j * 6, len * 6);
							}

							// down-place
							place = &uni.worlds[buff[i * size + j].id_world].down;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = (int)(size_y / 2.0f * (1 - 1 / just_now_buff.s)) + (int)(size_y / just_now_buff.s);

								vec2f *arr = arr_point + size_x * 18 * i + size_x * 12 + (j + k) * 6;

								arr[0] = { pos_x, y + pos_y };
								arr[1] = { pos_x, (float)y + size_y };
								arr[2] = { pos_x + 1, (float)y + size_y };
								arr[3] = { pos_x + 1, (float)y + size_y };
								arr[4] = { pos_x + 1, y + pos_y };
								arr[5] = { pos_x, y + pos_y };

								/*if (place->draw_type) { // sky?!
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								} else { // place
									if (place->type == 0) {
										arr[k * 6].color = arr[k * 6 + 1].color = arr[k * 6 + 2].color = arr[k * 6 + 3].color = arr[k * 6 + 4].color = arr[k * 6 + 5].color = \
											{ place->r, place->g, place->b, 255 };
									}
								}*/
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawArrays(GL_TRIANGLES, size_x * 18 * i + size_x * 12 + j * 6, len * 6);
							}
						}

						j += len;
					}

					time[i >> 3] ^= 1 << (i & 0x7);
				}
			}

			glUnmapBuffer(GL_ARRAY_BUFFER);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a) {
			vertex_line_arr;

			arr[0].color = arr[1].color = arr[2].color = arr[3].color = { r, g, b, a };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip);
		}

		void draw_image(Window &win, int x, int y, int size, Image &img, float t) {
			vertex_line_arr;

			sf::Vector2f pos = { img.texture.getSize().x * t, (float)img.texture.getSize().y };

			arr[0].texCoords = { pos.x - 1, 0 };
			arr[1].texCoords = { pos.x + 1, 0 };
			arr[2].texCoords = { pos.x - 1, pos.y };
			arr[3].texCoords = { pos.x + 1, pos.y };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip, &img.texture);
		}

		/*void draw_part_image(Window &win, int x, int y, int size, Image &img, float from_img_x, float from_img_y, float to_img_x, float to_img_y) {
			vertex_line_arr;

			arr[0].texCoords = { from_img_x, from_img_y };
			arr[1].texCoords = { to_img_x, from_img_y };
			arr[2].texCoords = { from_img_x, to_img_y };
			arr[3].texCoords = { to_img_x, to_img_y };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip, &img.texture);
		}*/
	}
}

