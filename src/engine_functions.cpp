#include "engine_functions.hpp"

#include <iostream>
#include <fstream>
#include <string.h>

#include <GL/glew.h>

#include "calc_draw.hpp"

using namespace pseudo3d_engine::math;

#define print_error_load(mes) std::cerr << "\033[31mError load map\033[39m: " << mes << std::endl; file.close(); return false;

std::istream& operator>>(std::istream &in, uchar &q) {
	unsigned int w;
	in >> w;
	q = w;
	return in;
}

namespace pseudo3d_engine {
	void draw_player_see(draw::Window &window, Universe &uni, MovingObject &player, Vec2i from, Vec2i to, float angl_see) {
		draw::init_buff(to.x - from.x);

		const float a_shift = angl_see / (to.x - from.x), start_a = player.a - angl_see / 2;

		for (int i = 0; i <= to.x - from.x; ++i)
			calc::draw_line(window, uni, player.id_world, player.pos, Vec2f::from_a(start_a + a_shift * i), i, from.y, to.y - from.y);

		draw::draw(window, uni, from.x, from.y, to.x - from.x, to.y - from.y);
	}

	uchar to_uchar(char q) {
		if ('a' <= q && q <= 'f')
			return q - 'a' + 10;
		else if ('A' <= q && q <= 'F')
			return q - 'A' + 10;
		return q - '0';
	}

	inline void ignore_space(std::istream &in) {
		while (in.peek() == ' ' || in.peek() == '\n')
			in.get();
	}

	inline void ignore_comment(std::istream &in) {
		ignore_space(in);
		while (in.peek() == '#')
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	bool load_color(std::ifstream &file, uchar &r, uchar &g, uchar &b, uchar &a, const char *where) {
		ignore_space(file);
		if (file.peek() == '#') { // like #00ffBB(ff)
			std::string color;
			file >> color;

			if (color.size() != 7 && color.size() != 9) {
				print_error_load("strange color on " << where);
			}

			r = to_uchar(color[1]) * 16 + to_uchar(color[2]);
			g = to_uchar(color[3]) * 16 + to_uchar(color[4]);
			b = to_uchar(color[5]) * 16 + to_uchar(color[6]);
			if (color.size() == 9)
				a = to_uchar(color[7]) * 16 + to_uchar(color[8]);
			else
				a = 255;
		} else { // like 0 255 187( 255)
			if (!(file >> r >> g >> b)) {
				print_error_load("r, g or b on color (" << where << ") isn't number");
			}

			if (file.peek() != '\n') {
				if (!(file >> a)) {
					print_error_load("alpha on color (" << where << " world) isn't number");
			}
		} else
			a = 255;
		}

		return true;
	}

	bool load_place_map(std::ifstream &file, Place &place, const char *name) {
		uchar tmp0, tmp1;
		if (!(file >> tmp0 >> tmp1)) {
			print_error_load("type of " << name << " isn't number");
		} else if (tmp0 > 2) {
			print_error_load("draw_type of " << name << " is too mach");
		}
		place.draw_type = tmp0;
		place.type = tmp1;

		switch (tmp1) {
		case 0:
			if (!load_color(file, place.r, place.g, place.b, tmp1, name))
				return false;
			break;
		default:
			print_error_load("don't know type of " << name);
		}
		return true;
	}

	bool load_universe_map(const char *path, Universe &ans) {
		std::ifstream file(path);

		if (!file.is_open()) {
			print_error_load("file \"" << path << "\" isn't exist");
		}

		// 0b - worlds, 1b - images
		uchar flags = 0;

		while (!file.eof()) {
			std::string str;
			if (!(file >> str))
				break;

			if (str[0] == '#') {
				while (!file.eof() && file.get() != '\n');
			} else if (str == "worlds:") {
				if (flags & 1) {
					print_error_load("worlds have already been");
				}
				flags |= 1;

				int size_worlds;
				if (!(file >> size_worlds)) {
					print_error_load("count of worlds isn't number");
				} else if (size_worlds < 0) {
					print_error_load("count of worlds is negative");
				}

				ans.set_worlds(size_worlds);

				for (int i = 0; i < size_worlds; ++i) {
					ignore_comment(file);

					// place
					if (!load_place_map(file, ans.worlds[i].up, "up-place"))
						return false;

					if (!load_place_map(file, ans.worlds[i].down, "down-place"))
						return false;

					// walls
					int count_walls;
					if (!(file >> count_walls)) {
						print_error_load("count of walls on " << i << " world isn't number");
					} else if (count_walls < 0) {
						print_error_load("count of walls on " << i << " world is negative");
					}

					ans.worlds[i].resize_walls(count_walls);

					for (int j = 0; j < count_walls; ++j) {
						ignore_comment(file);

						Wall &wall = ans.worlds[i].walls[j];

						if (!(file >> wall.from.x >> wall.from.y >> wall.a.x >> wall.a.y)) {
							print_error_load("can not load position " << j << " wall on " << i << " world");
						}

						wall.a -= wall.from;

						if (!wall.a.x && !wall.a.y) {
							print_error_load(j << " wall on " << i << " world isn't wall");
						}

						uchar tmp0, tmp1;
						char tmp2;
						if (!(file >> tmp0 >> tmp1 >> tmp2)) {
							print_error_load("can not load types of " << j << " wall on " << i << " world");
						}
						wall.type = tmp0;
						wall.draw_type = tmp1;
						if (tmp2 == 't')
							wall.phys_pass = 1;
						else if (tmp2 == 'f')
							wall.phys_pass = 0;
						else {
							print_error_load("don't know phys_pass \"" << tmp2 << "\", \"t\" or \"f\"");
						}

						if (wall.type > 3) {
							print_error_load("don't know " << (int)wall.type << " type on " << j << " wall, " << i << " world");
						}

						std::uint16_t id_wall_portal;
						uchar id_world_portal;
						if (wall.type == 3) {
							if (!(file >> id_world_portal >> id_wall_portal)) {
								print_error_load("no id_wall and id_world for portal on " << j << " wall " << i << " world");
							}
						}

						if (wall.draw_type == 0);
						else if (wall.draw_type == 1) {
							if (!load_color(file, wall.r, wall.g, wall.b, wall.alpha, "wall"))
								return false;
						} else if (wall.draw_type == 2) {
							if (!(file >> wall.id_texture)) {
								print_error_load("id texture on " << j << " wall on " << i << " world isn't number");
							}
						} else { //! add more
							print_error_load("don't know " << (int)wall.draw_type << " draw type on " << j << " wall on " << i << " world");
						}

						if (wall.type == 3) {
							if (wall.draw_type) {
								ans.worlds[i].resize_adata(ans.worlds[i].add_data_size + 1);
								add_data &adata = ans.worlds[i].adata[ans.worlds[i].add_data_size - 1];

								// if draw_type != 1, then uint = uint, else uint (uchar * 4) = uint (uchar * 4)
								adata.id_texture = wall.id_texture;
								adata.id_wall = id_wall_portal;
								adata.id_world = id_world_portal;

								wall.id_add_data = ans.worlds[i].add_data_size - 1;
							} else {
								wall.id_wall = id_wall_portal;
								wall.id_world = id_world_portal;
							}
						}
					}
				}
			} else if (str == "images:") {
				if (flags & 2) {
					print_error_load("images have already been");
				}
				flags |= 2;

				unsigned int count;
				if (!(file >> count)) {
					print_error_load("can not load count of image");
				}
				ans.resize_images(count);

				for (int i = 0; i < count; ++i) {
					ignore_comment(file);

					std::string path;

					if (!(file >> path)) {
						print_error_load("can not load path of " << i << " image");
					}

					if (!ans.images[i].load(path.c_str())) {
						print_error_load("error on load " << i << " image from \"" << path << '"');
					}
				}
			} else {
				print_error_load("don't know \"" << str << "\"");
			}
		}

		file.close();

		return true;
	}

	void save_place_map(std::ofstream &file, const Place &place) {
		file << (int)place.draw_type << ' ' << (int)place.type;

		switch (place.type) {
		case 0:
			file << ' ' << (int)place.r << ' ' << (int)place.g << ' ' << (int)place.b;
		}

		file << std::endl;
	}

	void save_universe_map(const char *path, const Universe &uni) {
		std::ofstream file(path);

		file << "worlds: " << (int)uni.size_worlds << std::endl;
		for (int i = 0; i < uni.size_worlds; ++i) {
			// place
			save_place_map(file, uni.worlds[i].up);
			save_place_map(file, uni.worlds[i].down);

			// walls
			file << uni.worlds[i].walls_size << std::endl;

			for (int j = 0; j < uni.worlds[i].walls_size; ++j) {
				Wall &wall = uni.worlds[i].walls[j];

				Vec2i to = wall.from + wall.a;

				file << wall.from.x << ' ' << wall.from.y << ' ' << to.x << ' ' << to.y << ' ' << \
				(int)wall.type << ' ' << (int)wall.draw_type;

				if (wall.phys_pass)
					file << " t";
				else
					file << " f";

				if (wall.type == 3) {
					if (wall.draw_type) {
						add_data &adata = uni.worlds[i].adata[wall.id_add_data];

						file << ' ' << (int)adata.id_world << ' ' << adata.id_wall;

						switch (wall.draw_type) {
						case 1:
							file << ' ' << (int)adata.r << ' ' << (int)adata.g << ' ' << (int)adata.b << ' ' << (int)adata.alpha;
							break;

						case 2:
							file << ' ' << adata.id_texture;
							break;
						}
					} else
						file << ' ' << (int)wall.id_world << ' ' << wall.id_wall;
				} else {
					switch (wall.draw_type) {
					case 1:
						file << ' ' << (int)wall.r << ' ' << (int)wall.g << ' ' << (int)wall.b << ' ' << (int)wall.alpha;
						break;

					case 2:
						file << ' ' << wall.id_texture;
						break;
					}
				}

				file << std::endl;
			}
		}

		file << "images: " << uni.size_image << std::endl;
		for (int i = 0; i < uni.size_image; ++i) {
			file << uni.images[i].path << std::endl;
		}

		file.close();
	}

	void to_buf(uchar *buf, unsigned int n) {
		for (int i = 0; i < 4; ++i) {
			buf[i] = (n & 0xff);
			n >>= 8;
		}
	}

	unsigned int from_buf_uint(uchar *buf) {
		unsigned int ans = 0;

		for (int i = 3; i >= 0; --i)
			ans = ((ans << 8) | buf[i]);

		return ans;
	}

	void to_buf(uchar *buf, int n) {
		bool neg = n < 0;
		if (neg)
			n = -n;

		for (int i = 0; i < 4; ++i) {
			buf[i] = (n & 0xff);
			n >>= 8;
		}

		buf[3] = (buf[3] << 1) | (uchar)neg;
	}

	int from_buf_int(uchar *buf) {
		int ans = buf[3];
		bool neg = (ans & 1);

		ans >>= 1;

		for (int i = 2; i >= 0; --i)
			ans = ((ans << 8) | buf[i]);

		if (neg)
			return -ans;
		return ans;
	}

	void to_buf(uchar *buf, std::uint16_t n) {
		buf[0] = n & 0xff;
		buf[1] = n >> 8;
	}

	inline std::uint16_t from_buf_16(uchar *buf) {
		return (buf[1] << 8) | buf[0];
	}

	// becouse 0 <= n <= 1
	// 0x0.ffff = 0x1.0 !
	void to_buf(uchar *buf, float n) {
		if (n == 1)
			buf[0] = buf[1] = 0xff;
		else
			to_buf(buf, (std::uint16_t)(n * 0x10000));
	}

	float from_buf_float(uchar *buf) {
		std::uint16_t ans = from_buf_16(buf);

		if (ans == 0xffff)
			return 1.0f;
		return ans / 0x10000f;
	}

	void save_place_binary(std::ofstream &file, uchar *buf, Place &place) {
		buf[0] = (place.draw_type << 7) | place.type;

		switch (place.type) {
		case 0:
			buf[1] = place.r;
			buf[2] = place.g;
			buf[3] = place.b;

			file.write((char*)buf, 4);
			break;
		}
	}

	bool load_place_binary(std::ifstream &file, uchar *buf, Place &place) {
		if (!file.read((char*)buf, 1)) {
			delete[] buf;
			print_error_load("no type of place");
		}

		place.draw_type = buf[0] >> 7;
		place.type = buf[0] & 0x8f;

		switch (place.type) {
		case 0:
			if (!file.read((char *)buf, 3)) {
				delete[] buf;
				print_error_load("no color of place");
			}

			place.r = buf[0];
			place.g = buf[1];
			place.b = buf[2];
			break;
		}

		return true;
	}

	bool load_universe_binary(std::ifstream &file, Universe &uni) {
		uchar *buf = new uchar[20];

		// worlds
		if (!file.read((char*)buf, 1)) {
			delete[] buf;
			print_error_load("no size of worlds");
		}
		int size_worlds = buf[0];

		uni.set_worlds(size_worlds);

		for (int i = 0; i < size_worlds; ++i) {
			// places
			if (!load_place_binary(file, buf, uni.worlds[i].up))
				return false;

			if (!load_place_binary(file, buf, uni.worlds[i].down))
				return false;

			// walls
			if (!file.read((char*)buf, 2)) {
				delete[] buf;
				print_error_load("no size of walls on " << i << " world");
			}
			uni.worlds[i].resize_walls(from_buf_16(buf));

			for (int j = 0; j < uni.worlds[i].walls_size; ++j) {
				Wall &wall = uni.worlds[i].walls[j];

				if (!file.read((char*)buf, 17)) {
					delete[] buf;
					print_error_load("no wall " << j << " on " << i << " world");
				}

				wall.from.x = from_buf_int(buf);
				wall.from.y = from_buf_int(buf + 4);
				wall.a.x = from_buf_int(buf + 8);
				wall.a.y = from_buf_int(buf + 12);
				wall.phys_pass = buf[16] & 1;
				wall.draw_type = (buf[16] >> 1) & 0xf;
				wall.type = (buf[16] >> 5);

				add_data *adata;
				if (wall.type == 3) {
					if (!file.read((char*)buf, 3)) {
						delete[] buf;
						print_error_load("no portal data on " << j << " wall on " << i << " world");
					}

					if (wall.draw_type) {
						uni.worlds[i].resize_adata(uni.worlds[i].add_data_size + 1);
						adata = &uni.worlds[i].adata[uni.worlds[i].add_data_size - 1];

						wall.id_add_data = uni.worlds[i].add_data_size - 1;

						adata->id_wall = from_buf_16(buf);
						adata->id_world = buf[2];
					} else {
						wall.id_wall = from_buf_16(buf);
						wall.id_world = buf[2];
					}
				}

				switch (wall.draw_type) {
				case 0:
					break;

				case 1:
					if (!file.read((char*)buf, 4)) {
						delete[] buf;
						print_error_load("no color for " << j << " wall on " << i << " world");
					}

					if (wall.type == 3) {
						adata->r = buf[0];
						adata->g = buf[1];
						adata->b = buf[2];
						adata->alpha = buf[3];
					} else {
						wall.r = buf[0];
						wall.g = buf[1];
						wall.b = buf[2];
						wall.alpha = buf[3];
					}
					break;

				case 2:
					if (!file.read((char*)buf, 4)) {
						delete[] buf;
						print_error_load("no id image for " << j << " wall on " << i << " world");
					}

					if (wall.type == 3)
						adata->id_texture = from_buf_uint(buf);
					else
						wall.id_texture = from_buf_uint(buf);
					break;

				default:
					delete[] buf;
					print_error_load("don't know " << (int)wall.draw_type << " type of " << j << " wall on " << i << " world");
				}
			}
		}

		// images
		if (!file.read((char*)buf, 4)) {
			delete[] buf;
			print_error_load("no count of images");
		}
		uni.resize_images(from_buf_uint(buf));

		char *buf_path;
		int j;
		if (uni.size_image) {
			buf_path = new char[257];
			buf_path[0] = 1;
		}

		for (int i = 0; i < uni.size_image; ++i) {
			for (j = 1; buf_path[j - 1]; ++j) {
				if (!file.read(buf_path + j, 1)) {
					delete[] buf;
					delete[] buf_path;
					print_error_load("uncorrect path on " << i << " image");
				}
			}

			uni.images[i].load(buf_path + 1);
		}

		if (uni.size_image)
			delete[] buf_path;

		delete[] buf;
		return true;
	}

	void save_universe_binary(std::ofstream &file, const Universe &uni) {
		uchar *buf = new uchar[20];

		// worlds
		buf[0] = uni.size_worlds;
		file.write((char*)buf, 1);

		for (int i = 0; i < uni.size_worlds; ++i) {
			// places
			save_place_binary(file, buf, uni.worlds[i].up);
			save_place_binary(file, buf, uni.worlds[i].down);

			// walls
			to_buf(buf, uni.worlds[i].walls_size);
			file.write((char*)buf, 2);

			for (int j = 0; j < uni.worlds[i].walls_size; ++j) {
				Wall &wall = uni.worlds[i].walls[j];

				to_buf(buf, wall.from.x);
				to_buf(buf + 4, wall.from.y);
				to_buf(buf + 8, wall.a.x);
				to_buf(buf + 12, wall.a.y);
				buf[16] = (wall.type << 5) | (wall.draw_type << 1) | wall.phys_pass;

				file.write((char*)buf, 17);

				add_data *adata;
				if (wall.type == 3) {
					if (wall.draw_type) {
						adata = &uni.worlds[i].adata[wall.id_add_data];

						to_buf(buf, adata->id_wall);
						buf[2] = adata->id_world;
					} else {
						to_buf(buf, wall.id_wall);
						buf[2] = wall.id_world;
					}

					file.write((char*)buf, 3);
				}

				switch (wall.draw_type) {
				case 1:
					if (wall.type == 3) {
						buf[0] = adata->r;
						buf[1] = adata->g;
						buf[2] = adata->b;
						buf[3] = adata->alpha;
					} else {
						buf[0] = wall.r;
						buf[1] = wall.g;
						buf[2] = wall.b;
						buf[3] = wall.alpha;
					}

					file.write((char*)buf, 4);
					break;

				case 2:
					if (wall.type == 3)
						to_buf(buf, adata->id_texture);
					else
						to_buf(buf, wall.id_texture);

					file.write((char*)buf, 4);
					break;
				}
			}
		}

		// images
		to_buf(buf, uni.size_image);
		file.write((char*)buf, 4);

		for (int i = 0; i < uni.size_image; ++i)
			file.write(uni.images[i].path, uni.images[i].size + 1);

		delete[] buf;
	}

	void save_universe_mapb(const char *path, const Universe &uni) {
		std::ofstream file(path, std::ios::binary);

		save_universe_binary(file, uni);

		file.close();
	}

	bool load_universe_mapb(const char *path, Universe &ans) {
		std::ifstream file(path, std::ios::binary);

		if (!file.is_open()) {
			print_error_load("no \"" << path << "\" file");
		}

		if (load_universe_binary(file, ans)) {
			file.close();
			return true;
		}
		return false;
	}

	bool load_universe_mapo(const char *path, Universe &ans) {
		std::ifstream file(path, std::ios::binary);

		if (!file.is_open()) {
			print_error_load("no \"" << path << "\" file");
		}

		if (!load_universe_binary(file, ans)) {
			return false;
		}

		uchar *buf = new uchar[30];

		for (int i = 0; i < ans.size_worlds; ++i) {
			World &world = ans.worlds[i];

			if (!file.read((char*)buf, 2)) {
				print_error_load("no node_size on " << i << " world");
			}
			world.resize_nodes(from_buf_16(buf));

			for (int j = 0; j < world.node_size; ++j) {
				if (!file.read((char*)buf, 6)) {
					print_error_load("no " << j << " node on " << i << " world");
				}

				world.nodes[j].id_wall = from_buf_16(buf);
				world.nodes[j].left = from_buf_16(buf + 2);
				world.nodes[j].right = from_buf_16(buf + 4);
			}

			if (!file.read((char*)buf, 2)) {
				print_error_load("no wall_ptr_size on " << i << " world");
			}
			world.resize_pwalls(from_buf_16(buf));

			for (int j = 0; j < world.wall_ptr_size; ++j) {
				for (int k = 0; k < 4; ++k) {
					if (!file.read((char*)buf, 2)) {
						print_error_load("no " << j << ':' << k << " pwall on " << i << " world");
					}
					world.pwalls[j * 4 + k].t_end = from_buf_float(buf);

					if (world.pwalls[j * 4 + k].t_end == 0)
						break;

					if (!file.read((char*)buf, 4)) {
						print_error_load("where are full " << j << ':' << k << " pwall on " << i << " world?!");
					}

					world.pwalls[j * 4 + k].t_start = from_buf_float(buf);
					world.pwalls[j * 4 + k].id_wall = from_buf_16(buf + 2);
				}
			}
		}

		delete[] buf;
		file.close();
		return true;
	}

	void save_universe_mapo(const char *path, const Universe &uni) {
		std::ofstream file(path, std::ios::binary);

		save_universe_binary(file, uni);

		uchar *buf = new uchar[30];

		// binary tree
		for (int i = 0; i < uni.size_worlds; ++i) {
			World &world = uni.worlds[i];

			to_buf(buf, world.node_size);
			file.write((char*)buf, 2);

			for (int j = 0; j < world.node_size; ++j) {
				to_buf(buf, world.nodes[j].id_wall);
				to_buf(buf + 2, world.nodes[j].left);
				to_buf(buf + 4, world.nodes[j].right);

				file.write((char*)buf, 6);
			}

			to_buf(buf, world.wall_ptr_size);
			file.write((char*)buf, 2);

			for (int j = 0; j < world.wall_ptr_size; ++j) {
				uchar size = 0;
				for (int k = 0; k < 4; ++k) {
					to_buf(buf + 6 * k, world.pwalls[j * 4 + k].t_end);

					if (world.pwalls[j * 4 + k].t_end == 0) {
						size += 2;
						break;
					}

					to_buf(buf + 6 * k + 2, world.pwalls[j * 4 + k].t_start);
					to_buf(buf + 6 * k + 4, world.pwalls[j * 4 + k].id_wall);

					size += 6;
				}

				file.write((char*)buf, size);
			}
		}

		delete[] buf;
		file.close();
	}

	bool load_universe(const char *path, Universe &uni, bool make_tree) {
		int i = 0;
		while (path[i]) ++i;

		if (i < 5) {
			std::cerr << "\033[31mError load\033[39m: don't know type" << std::endl;
			return false;
		}

		if (!strcmp(path + i - 4, ".map")) {
			if (!load_universe_map(path, uni))
				return false;
		} else if (!strcmp(path + i - 5, ".mapb")) {
			if (!load_universe_mapb(path, uni))
				return false;
		} else if (!strcmp(path + i - 5, ".mapo")) {
			if (!load_universe_mapo(path, uni))
				return false;
			make_tree = false;
		} else {
			std::cerr << "\033[31mError load\033[39m: don't know type" << std::endl;
			return false;
		}

		if (make_tree)
			for (int i = 0; i < uni.size_worlds; ++i)
				uni.worlds[i].make_tree();

		return true;
	}

	void save_universe(const char *path, const Universe &uni) {
		int i = 0;
		while (path[i]) ++i;

		if (i >= 5) {
			if (!strcmp(path + i - 4, ".map")) {
				save_universe_map(path, uni);
				return;
			} else if (!strcmp(path + i - 5, ".mapb")) {
				save_universe_mapb(path, uni);
				return;
			} else if (!strcmp(path + i - 5, ".mapo")) {
				save_universe_mapo(path, uni);
				return;
			}
		}

		char *new_path = new char[i + 4 + 1];
		for (int j = 0; path[j]; ++j)
			new_path[j] = path[j];

		new_path[i] = '.';
		new_path[i + 1] = 'm';
		new_path[i + 2] = 'a';
		new_path[i + 3] = 'p';
		new_path[i + 4] = 0;

		save_universe_map(new_path, uni);

		delete[] new_path;
	}

	bool init(const char *path_settings) {
		if (path_settings == nullptr) {
			get_settings().depth_draw = 16;
		} else {
			std::ifstream file(path_settings);
			if (!file.is_open()) {
				std::cerr << "\033[31mError load settings\033[39m: no file \"" << path_settings << '"' << std::endl;
				return false;
			}

			if (!(file >> get_settings().depth_draw)) {
				std::cerr << "\033[31Error load settings\033[39m: depth_draw is not a number" << std::endl;
				file.close();
				return false;
			}

			if (get_settings().depth_draw < 2) {
				std::cerr << "\033[31mError load settings\033[39m: depth_draw < 2!" << std::endl;
				file.close();
				return false;
			}

			file.close();
		}

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			std::cerr << "\033[31mError load GLEW\033[39m" << std::endl;
			return false;
		}

		if (!draw::init())
			return false;

		return true;
	}

	void deinit() {
		draw::deinit();
	}

	void save_settings(const char *path) {
		std::ofstream file(path);

		file << (int)get_settings().depth_draw << std::endl;

		file.close();
	}
}
