#include "draw.hpp"

#include <stdlib.h>
#include <fstream>
#include <iostream>

#include "stb_image.h"

#include "engine_structs.hpp"
#include "my_shaders.hpp"

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

			stbi_set_flip_vertically_on_load(1);

			int chan;
			uchar *data = stbi_load(path, &x, &y, &chan, 4);

			if (data == nullptr)
				return false;

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);

			return true;
		}

		void Image::unload() {
			glDeleteTextures(1, &texture);
			delete[] path;
		}

		int Image::get_x() {
			return x;
		}

		int Image::get_y() {
			return y;
		}

		// shader's functions
		bool load_shader(GLuint type, const char *text, GLuint &ans) {
			/*std::ifstream file(path);
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

			file.close();*/

			ans = glCreateShader(type);
			glShaderSource(ans, 1, &text, nullptr);
			glCompileShader(ans);

			GLint ret;
			glGetShaderiv(ans, GL_COMPILE_STATUS, &ret);
			if (!ret) {
				char text[512];
				glGetShaderInfoLog(ans, 512, nullptr, text);
				std::cerr << "\033[31mError compile shader\033[39m: " << text << std::endl;

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
		static GLuint prog_color = 0, prog_image = 0, prog_floor = 0, prog_floor_inf = 0;
		static GLuint vbo = 0, ebo = 0;

		bool init() {
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ebo);

			GLuint sh_vec;
			if (!load_shader(GL_VERTEX_SHADER, SHADER_VERTEX, sh_vec))
				return false;

			// prog_color
			GLuint sh_color;
			if (!load_shader(GL_FRAGMENT_SHADER, SHADER_COLOR, sh_color)) {
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

			// prog_image
			GLuint sh_image;
			if (!load_shader(GL_FRAGMENT_SHADER, SHADER_IMAGE, sh_image)) {
				glDeleteShader(sh_vec);
				return false;
			}

			prog_image = glCreateProgram();
			glAttachShader(prog_image, sh_vec);
			glAttachShader(prog_image, sh_image);

			glLinkProgram(prog_image);
			if (!check_link_prog(prog_image)) {
				glDeleteShader(sh_vec);
				glDeleteShader(sh_color);
				return false;
			}

			glDeleteShader(sh_image);

			// prog_floor
			GLuint sh_floor;
			if (!load_shader(GL_FRAGMENT_SHADER, SHADER_FLOOR, sh_floor)) {
				glDeleteShader(sh_vec);
				return false;
			}

			prog_floor = glCreateProgram();
			glAttachShader(prog_floor, sh_vec);
			glAttachShader(prog_floor, sh_floor);

			glLinkProgram(prog_floor);
			if (!check_link_prog(prog_floor)) {
				glDeleteShader(sh_vec);
				glDeleteShader(sh_floor);
				return false;
			}

			glDeleteShader(sh_floor);

			// prog_floor_inf
			GLuint sh_floor_inf;
			if (!load_shader(GL_FRAGMENT_SHADER, SHADER_FLOOR_INF, sh_floor_inf)) {
				glDeleteShader(sh_vec);
				return false;
			}

			prog_floor_inf = glCreateProgram();
			glAttachShader(prog_floor_inf, sh_vec);
			glAttachShader(prog_floor_inf, sh_floor_inf);

			glLinkProgram(prog_floor_inf);
			if (!check_link_prog(prog_floor_inf)) {
				glDeleteShader(sh_vec);
				glDeleteShader(sh_floor_inf);
				return false;
			}

			glDeleteShader(sh_floor_inf);

			glDeleteShader(sh_vec);

			return true;
		}

		// buffer
		struct buffer_draw {
			float s_old, s, t;
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
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (size_new != size) {
				std::free(buff);
				std::free(use);
				std::free(time);

				buff = (buffer_draw*)std::calloc(size_new * get_settings().depth_draw, sizeof(buffer_draw));
				use = (uchar*)std::calloc((get_settings().depth_draw >> 3) + 1, sizeof(uchar));
				time = (uchar*)std::calloc((get_settings().depth_draw >> 3) + 1, sizeof(uchar));

				if (buff == nullptr || use == nullptr || time == nullptr)
					throw "little memory";

				size = size_new;
			} else {
				for (int i = get_settings().depth_draw >> 3; i > -1; --i)
					use[i] = 0;
			}
		}

                void deinit() {
			std::free(buff);
			std::free(use);
			std::free(time);

			if (prog_color) glDeleteProgram(prog_color);
			if (prog_image) glDeleteProgram(prog_image);
			if (prog_floor) glDeleteProgram(prog_floor);
			if (prog_floor_inf) glDeleteProgram(prog_floor_inf);

			if (vbo) glDeleteBuffers(1, &vbo);
			if (ebo) glDeleteBuffers(1, &ebo);
		}

                void add(unsigned int x, uchar depth, float s_old, float s, float t, float from_x, float from_y, float a_x, float a_y, std::uint16_t id_wall, uchar id_world) {
			use[depth >> 3] |= 1 << (depth & 0x7);

			buffer_draw &now = buff[depth * size + x];
			now.s_old = s_old;
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

		void add_none(unsigned int x, uchar depth, float s_old, float from_x, float from_y, float a_x, float a_y, uchar id_world) {
			use[depth >> 3] |= 1 << (depth & 0x7);

			buffer_draw &now = buff[depth * size + x];
			now.s_old = s_old;
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

		struct vec3i {
			GLuint x, y, z;
		};

		inline void draw_wall(Window &win, Universe &uni, int i, int j, int x, int y, int size_x, int size_y, const buffer_draw &buff_now, vec2f *arr_point, vec3i *arr_tri, int len) {
			const Wall &wall = uni.worlds[buff_now.id_world].walls[buff_now.id_wall];
			if (wall.draw_type) {
				for (int k = 0; k < len; ++k) {
					const buffer_draw &buff_just_now = buff[i * size + j + k];
					const float x_now = x + j + k,
						    y_now = y + size_y / 2.0 * (1 - 1 / buff_just_now.s);

					const unsigned int id = size_x * 20 + size_x * 60 * i + (j + k) * 20;
					vec3i *arr_ = arr_tri + size_x * 2 + size_x * 6 * i + (j + k) * 2;

					arr_point[id] = { x_now, y_now };
					arr_point[id + 5] = { x_now + 1, y_now };
					arr_point[id + 10] = { x_now + 1, y_now + size_y / buff_just_now.s };
					arr_point[id + 15] = { x_now, y_now + size_y / buff_just_now.s };

					const unsigned int id_ = id / 5;

					arr_[0] = { id_, id_ + 1, id_ + 2 };
					arr_[1] = { id_, id_ + 2, id_ + 3 };

					if (wall.draw_type == 2) {
						Image *img;
						if (wall.type == 3)
							img = &uni.images[uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].id_texture];
						else
							img = &uni.images[wall.id_texture];
						const float delta = 0.5f / img->get_x();

						arr_point[id + 1] = { buff_just_now.t - delta, 0 };
						arr_point[id + 6] = { buff_just_now.t + delta, 0 };
						arr_point[id + 11] = { buff_just_now.t + delta, 1 };
						arr_point[id + 16] = { buff_just_now.t - delta, 1 };
					}
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

					glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 24 + size_x * 72 * i + j * 24));
				} else if (wall.draw_type == 2) {
					GLint size_uni = glGetUniformLocation(prog_image, "size_screen");

					glUseProgram(prog_image);

					glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
					if (wall.type == 3)
						glBindTexture(GL_TEXTURE_2D, uni.images[uni.worlds[buff[i * size + j].id_world].adata[wall.id_add_data].id_texture].texture);
					else
						glBindTexture(GL_TEXTURE_2D, uni.images[wall.id_texture].texture);

					glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 24 + size_x * 72 * i + j * 24));

					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}
		}

                void draw(Window &win, Universe &uni, int x, int y, int size_x, int size_y) {
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, get_settings().depth_draw * size_x * 3 * 4 * 10 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, get_settings().depth_draw * size_x * 3 * 6 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

			vec2f *arr_point = (vec2f*)glMapBufferRange(GL_ARRAY_BUFFER, 0, get_settings().depth_draw * size_x * 3 * 4 * 10 * sizeof(GLfloat), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
			vec3i *arr_tri = (vec3i*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, get_settings().depth_draw * size_x * 3 * 6 * sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

			if (!arr_point || !arr_tri) {
				while (GLint er = glGetError())
					std::cerr << "\033[31mOpenGL error\033[39m: " << er << std::endl;
				return;
			}

			for (int i = 0; i < 5; ++i) {
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)((i << 1) * sizeof(GLfloat)));
			}

			for (int i = get_settings().depth_draw - 1; i > -1; --i) {
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
								float pos_y_old = 0;
								if (just_now_buff.s_old >= 1)
									pos_y_old = size_y / 2.0f * (1 - 1 / just_now_buff.s_old);

								const unsigned int id = size_x * 60 * i + (j + k) * 20;
								vec3i *arr_ = arr_tri + size_x * 6 * i + (j + k) * 2;

								arr_point[id] = { pos_x, y + pos_y_old };
								arr_point[id + 5] = { pos_x + 1, y + pos_y_old };
								arr_point[id + 10] = { pos_x + 1, y + pos_y };
								arr_point[id + 15] = { pos_x, y + pos_y };

								const unsigned int id_ = id / 5;

								arr_[0] = { id_, id_ + 1 , id_ + 2 };
								arr_[1] = { id_, id_ + 2, id_ + 3 };

								if (place->type == 1) {
									if (!place->draw_type) {
										arr_point[id + 1] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 2] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 3] = { just_now_buff.s_old, 1 };

										arr_point[id + 6] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 7] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 8] = { just_now_buff.s_old, 1 };

										arr_point[id + 11] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 12] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 13] = { just_now_buff.s_old, 0 };

										arr_point[id + 16] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 17] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 18] = { just_now_buff.s_old, 0 };
									}
								}
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 72 * i + j * 24));
							} else if (place->type == 1) {
								if (!place->draw_type) {
									GLuint size_uni = glGetUniformLocation(prog_floor_inf, "size_screen");

									glUseProgram(prog_floor_inf);

									glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
									glBindTexture(GL_TEXTURE_2D, uni.images[place->id_texture].texture);

									glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 72 * i + j * 24));

									glBindTexture(GL_TEXTURE_2D, 0);
								}
							}

							// down-place
							place = &uni.worlds[buff[i * size + j].id_world].down;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = size_y / 2;
								float pos_y_old;
								if (just_now_buff.s_old < 1)
									pos_y_old = size_y;
								else
									pos_y_old = (int)(size_y / 2.0f * (1 - 1 / just_now_buff.s_old)) + (int)(size_y / just_now_buff.s_old);

								const unsigned int id = size_x * 40 + size_x * 60 * i + (j + k) * 20;
								vec3i *arr_ = arr_tri + size_x * 4 + size_x * 6 * i + (j + k) * 2;

								arr_point[id] = { pos_x, y + pos_y };
								arr_point[id + 5] = { pos_x + 1, y + pos_y };
								arr_point[id + 10] = { pos_x + 1, y + pos_y_old };
								arr_point[id + 15] = { pos_x, y + pos_y_old };

								const unsigned int id_ = id / 5;

								arr_[0] = { id_, id_ + 1, id_ + 2 };
								arr_[1] = { id_, id_ + 2, id_ + 3 };

								if (place->type == 1) {
									if (!place->draw_type) {
										arr_point[id + 1] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 2] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 3] = { just_now_buff.s_old, 0 };

										arr_point[id + 6] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 7] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 8] = { just_now_buff.s_old, 0 };

										arr_point[id + 11] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 12] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 13] = { just_now_buff.s_old, 1 };

										arr_point[id + 16] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 17] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 18] = { just_now_buff.s_old, 1 };
									}
								}
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 48 + size_x * 72 * i + j * 24));
							} else if (place->type == 1) {
								if (!place->draw_type) {
									GLuint size_uni = glGetUniformLocation(prog_floor_inf, "size_screen");

									glUseProgram(prog_floor_inf);

									glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
									glBindTexture(GL_TEXTURE_2D, uni.images[place->id_texture].texture);

									glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 48 + size_x * 72 * i + j * 24));

									glBindTexture(GL_TEXTURE_2D, 0);
								}
							}
						} else {
							// draw normal
							const buffer_draw &buff_now = buff[i * size + j];

							// walls
							draw_wall(win, uni, i, j, x, y, size_x, size_y, buff_now, arr_point, arr_tri, len);

							// up-place
							Place *place = &uni.worlds[buff[i * size + j].id_world].up;
							for (int k = 0; k < len; ++k) {
								const buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = size_y / 2.0f * (1 - 1 / just_now_buff.s);
								float pos_y_old = 0;
								if (just_now_buff.s_old >= 1)
									pos_y_old = size_y / 2.0f * (1 - 1 / just_now_buff.s_old);

								const unsigned int id = size_x * 60 * i + (j + k) * 20;
								vec3i *arr_ = arr_tri + size_x * 6 * i + (j + k) * 2;

								arr_point[id] = { pos_x, y + pos_y_old };
								arr_point[id + 5] = { pos_x + 1, y + pos_y_old };
								arr_point[id + 10] = { pos_x + 1, y + pos_y };
								arr_point[id + 15] = { pos_x, y + pos_y };

								const unsigned int id_ = id / 5;

								arr_[0] = { id_, id_ + 1, id_ + 2 };
								arr_[1] = { id_, id_ + 2, id_ + 3 };

								if (place->type == 1) {
									if (!place->draw_type) {
										arr_point[id + 1] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 2] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 3] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 4] = { 1, 0 };

										arr_point[id + 6] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 7] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 8] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 9] = { 1, 0 };

										arr_point[id + 11] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 12] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 13] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 14] = { 0, 0 };

										arr_point[id + 16] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 17] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 18] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 19] = { 0, 0 };
									}
								}
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 72 * i + j * 24));
							} else if (place->type == 1) {
								if (!place->draw_type) {
									GLuint size_uni = glGetUniformLocation(prog_floor, "size_screen");

									glUseProgram(prog_floor);

									glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
									glBindTexture(GL_TEXTURE_2D, uni.images[place->id_texture].texture);

									glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 72 * i + j * 24));

									glBindTexture(GL_TEXTURE_2D, 0);
								}
							}

							// down-place
							place = &uni.worlds[buff[i * size + j].id_world].down;
							for (int k = 0; k < len; ++k) {
								buffer_draw &just_now_buff = buff[i * size + j + k];
								const float pos_x = x + j + k, pos_y = (int)(size_y / 2.0f * (1 - 1 / just_now_buff.s)) + (int)(size_y / just_now_buff.s);
								float pos_y_old;
								if (just_now_buff.s_old < 1)
									pos_y_old = size_y;
								else
									pos_y_old = (int)(size_y / 2.0f * (1 - 1 / just_now_buff.s_old)) + (int)(size_y / just_now_buff.s_old);

								const unsigned int id = size_x * 40 + size_x * 60 * i + (j + k) * 20;
								vec3i *arr_ = arr_tri + size_x * 4 + size_x * 6 * i + (j + k) * 2;

								arr_point[id] = { pos_x, y + pos_y };
								arr_point[id + 5] = { pos_x + 1, y + pos_y };
								arr_point[id + 10] = { pos_x + 1, y + pos_y_old };
								arr_point[id + 15] = { pos_x, y + pos_y_old };

								const unsigned int id_ = id / 5;

								arr_[0] = { id_, id_ + 1, id_ + 2 };
								arr_[1] = { id_, id_ + 2, id_ + 3 };

								if (place->type == 1) {
									if (!place->draw_type) {
										arr_point[id + 1] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 2] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 3] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 4] = { 0, 0 };

										arr_point[id + 6] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 7] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 8] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 9] = { 0, 0 };

										arr_point[id + 11] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 12] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 13] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 14] = { 1, 0 };

										arr_point[id + 16] = { just_now_buff.from_x, just_now_buff.from_y };
										arr_point[id + 17] = { just_now_buff.a_x, just_now_buff.a_y };
										arr_point[id + 18] = { just_now_buff.s_old, just_now_buff.s };
										arr_point[id + 19] = { 1, 0 };
									}
								}
							}

							if (place->type == 0) {
								GLint size_uni = glGetUniformLocation(prog_color, "size_screen");
								GLint color = glGetUniformLocation(prog_color, "color_");

								glUseProgram(prog_color);

								glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
								glUniform4f(color, place->r, place->g, place->b, 255);

								glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 48 + size_x * 72 * i + j * 24));
							} else if (place->type == 1) {
								if (!place->draw_type) {
									GLuint size_uni = glGetUniformLocation(prog_floor, "size_screen");

									glUseProgram(prog_floor);

									glUniform2f(size_uni, win.window->getSize().x, win.window->getSize().y);
									glBindTexture(GL_TEXTURE_2D, uni.images[place->id_texture].texture);

									glDrawElements(GL_TRIANGLES, len * 6, GL_UNSIGNED_INT, (void*)(size_x * 48 + size_x * 72 * i + j * 24));

									glBindTexture(GL_TEXTURE_2D, 0);
								}
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
	}
}

