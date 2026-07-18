#include "calc_draw.hpp"

namespace pseudo3d_engine {
	namespace calc {
		bool interact_with_wall(Wall &wall, float t_start, float t_end, math::Vec2f from, math::Vec2f a, float &s, float &t) {
			if (-EPS < wall.a.x && wall.a.x < EPS) { // wall.a.x == 0
				if (-EPS < a.x && a.x < EPS) // a.x == 0
					return false;
				else {
					const math::Vec2f delta = from - (math::Vec2f)wall.from;

					s = -delta.x / a.x;
					t = delta.y / wall.a.y - (delta.x * a.y) / (a.x * wall.a.y);

					return t_start <= t && t <= t_end && s > 0;
				}
			} else {
				const float tmp = a.y * wall.a.x - a.x * wall.a.y;
				if (-EPS < tmp && tmp < EPS) // tmp == 0
					return false;
				// else

				const math::Vec2f delta = from - (math::Vec2f)wall.from;

				s = (delta.x * wall.a.y - delta.y * wall.a.x) / tmp;
				t = (delta.x + s * a.x) / wall.a.x;

				return t_start <= t && t <= t_end && s > 0;
			}
		}

		bool check_node(Wall *walls, wall_ptr *pwalls, math::Vec2f from, math::Vec2f a, Wall *&wall, float &s, float &t) {
			float s_loc, t_loc;
			s = 100;

			for (int i = 0; i < 4 && pwalls[i].t_end; ++i)
				if (interact_with_wall(walls[pwalls[i].id_wall], pwalls[i].t_start, pwalls[i].t_end, from, a, s_loc, t_loc))
					if (s_loc < s) {
						s = s_loc;
						t = t_loc;
						wall = &walls[pwalls[i].id_wall];
					}

			return wall != nullptr;
		}

		void real_draw_line(draw::Window &window, Universe &uni, Wall *wall, float s, float t, int x, int y, int size_y) {
			switch (wall->draw_type) {
			case 0:
				break;
			case 1:
				draw::draw_line(window, x, (int)(size_y / 2.0 * (1 - 1 / s)), (int)(size_y / s), wall->r, wall->g, wall->b, wall->alpha);
				break;
			}
		}

		struct node_mem {
			std::uint16_t id;
			// 0b - left, 1b - right
			std::uint16_t flag = 0;
		};

		void draw_line(draw::Window &window, Universe &uni, uchar id_world, math::Vec2f from, math::Vec2f a, int x, int y, int size_y) {
			node_mem *mem = new node_mem[uni.worlds[id_world].get_size_tree() + 1];
			int i = 0;

			mem[i].id = mem[i].flag = 0;

			World &world = uni.worlds[id_world];

			while (i > -1) {
				if (mem[i].id & 0x8000) { // array
					float s, t;
					Wall *ans = nullptr;

					if (check_node(world.walls, world.pwalls + (((int)(mem[i].id & 0x7fff)) << 2), from, a, ans, s, t)) {
						switch (ans->type) {
						case 0:
							//! back stack_draw

							real_draw_line(window, uni, ans, s, t, x, y, size_y);
							delete[] mem;
							return;
						//! else: save to stack_draw
						}
					}

					--i;
				} else if (!mem[i].flag) { // chose left or right
					Wall &wall = world.walls[world.nodes[mem[i].id].id_wall];
					if (math::cross(wall.a, from - (math::Vec2f)wall.from) > 0) {
						mem[i].flag = 1;

						mem[i + 1].id = world.nodes[mem[i].id].left;
						mem[++i].flag = 0;
					} else {
						mem[i].flag = 2;

						mem[i + 1].id = world.nodes[mem[i].id].right;
						mem[++i].flag = 0;
					}
				} else if (mem[i].flag == 1) { // go right last
					mem[i].id = world.nodes[mem[i].id].right;
					mem[i].flag = 0;
				} else { // go left last
					mem[i].id = world.nodes[mem[i].id].left;
					mem[i].flag = 0;
				}
			}

			delete[] mem;
		}
	}
}
