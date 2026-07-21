#include "engine_functions.hpp"

#include <iostream>
#include <fstream>

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
	void draw_player_see(draw::Window &window, Universe &uni, uchar id_world, Player &player, Vec2i from, Vec2i to, float angl_see) {
		const float a_shift = angl_see / (to.x - from.x), start_a = player.a - angl_see / 2;

		for (int i = 0; i <= to.x - from.x; ++i)
			calc::draw_line(window, uni, id_world, player.pos, Vec2f::from_a(start_a + a_shift * i), i, from.y, to.y - from.y);
	}

	uchar to_uchar(char q) {
		if ('a' <= q && q <= 'f')
			return q - 'a' + 10;
		else if ('A' <= q && q <= 'F')
			return q - 'A' + 10;
		return q - '0';
	}

	void ignore_space(std::istream &in) {
		while (in.peek() == ' ' || in.peek() == '\n')
			in.get();
	}

	bool load_universe_map(const char *path, Universe &ans) {
		std::ifstream file(path);

		if (!file.is_open()) {
			print_error_load("file \"" << path << "\" isn't exist");
		}

		// 0b - worlds
		uchar flags = 0;

		while (!file.eof()) {
			std::string str;
			if (!(file >> str))
				break;

			if (str[0] == '#')
				continue;
			else if (str == "worlds:") {
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

				for (int i = 0; i < size_worlds; ++i) {
					ignore_space(file);
					while (file.peek() == '#')
						file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

					int count_walls;
					if (!(file >> count_walls)) {
						print_error_load("count of walls on " << i << " world isn't number");
					} else if (count_walls < 0) {
						print_error_load("count of walls on " << i << " world is negative");
					}

					ans.add_world(count_walls);

					for (int j = 0; j < count_walls; ++j) {
						ignore_space(file);
						while (file.peek() == '#')
							file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

						Wall &wall = ans.worlds[i].walls[j];

						if (!(file >> wall.from.x >> wall.from.y >> wall.a.x >> wall.a.y)) {
							print_error_load("can not load position " << j << " wall on " << i << " world");
						}

						wall.a -= wall.from;

						if (!wall.a.x && !wall.a.y) {
							print_error_load(j << " wall on " << i << " world isn't wall");
						}

						uchar tmp0, tmp1;
						if (!(file >> tmp0 >> tmp1)) {
							print_error_load("can not load types of " << j << " wall on " << i << " world");
						}
						wall.type = tmp0;
						wall.draw_type = tmp1;

						if (wall.type > 3) {
							print_error_load("don't know " << (int)wall.type << " type on " << j << " wall, " << i << " world");
						}

						if (wall.draw_type == 0);
						else if (wall.draw_type == 1) {
							ignore_space(file);
							if (file.peek() == '#') { // like #00ffBB(ff)
								std::string color;
								file >> color;

								if (color.size() != 7 && color.size() != 9) {
									print_error_load("strange color on " << j << " wall on " << i << " world");
								}

								wall.r = to_uchar(color[1]) * 16 + to_uchar(color[2]);
								wall.g = to_uchar(color[3]) * 16 + to_uchar(color[4]);
								wall.b = to_uchar(color[5]) * 16 + to_uchar(color[6]);
								if (color.size() == 9)
									wall.alpha = to_uchar(color[7]) * 16 + to_uchar(color[8]);
								else
									wall.alpha = 255;
							} else { // like 0 255 187( 255)
								if (!(file >> wall.r >> wall.g >> wall.b)) {
									print_error_load("r, g or b on color (" << i << " wall, " << j << " world) isn't number");
								}

								if (file.peek() != '\n') {
									if (!(file >> wall.alpha)) {
										print_error_load("alpha on color (" << i << " wall, " << j << " world) isn't number");
									}
								} else
									wall.alpha = 255;
							}
						} else { //! add more
							print_error_load("don't know " << (int)wall.draw_type << " draw type on " << j << " wall on " << i << " world");
						}
					}
				}
			} else {
				print_error_load("don't know \"" << str << "\"");
			}
		}

		file.close();

		return true;
	}

	void save_universe_map(const char *path, const Universe &uni) {
		std::ofstream file(path);

		file << "world: " << (int)uni.size_worlds << std::endl;
		for (int i = 0; i < uni.size_worlds; ++i) {
			file << uni.worlds[i].walls_size << std::endl;

			for (int j = 0; j < uni.worlds[i].walls_size; ++j) {
				Wall &wall = uni.worlds[i].walls[j];

				file << wall.from.x << ' ' << wall.from.y << ' ' << wall.a.x << ' ' << wall.a.y << ' ' << \
				(int)wall.type << ' ' << (int)wall.draw_type;

				switch (wall.draw_type) {
				case 0:
					break;

				case 1:
					file << ' ' << (int)wall.r << ' ' << (int)wall.g << ' ' << (int)wall.b << ' ' << (int)wall.alpha;
				}

				file << std::endl;
			}
		}

		file.close();
	}
}
