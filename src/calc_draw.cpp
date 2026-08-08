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

					return t_start <= t && t <= t_end && s > EPS;
				}
			} else {
				const float tmp = a.y * wall.a.x - a.x * wall.a.y;
				if (-EPS < tmp && tmp < EPS) // tmp == 0
					return false;
				// else

				const math::Vec2f delta = from - (math::Vec2f)wall.from;

				s = (delta.x * wall.a.y - delta.y * wall.a.x) / tmp;
				t = (delta.x + s * a.x) / wall.a.x;

				return t_start <= t && t <= t_end && s > EPS;
			}
		}

		bool check_node(Wall *walls, wall_ptr *pwalls, math::Vec2f from, math::Vec2f a, std::uint16_t &id_wall, float &s, float &t) {
			float s_loc, t_loc;
			bool find = false;
			s = 100;

			for (int i = 0; i < 4 && pwalls[i].t_end; ++i)
				if (interact_with_wall(walls[pwalls[i].id_wall], pwalls[i].t_start, pwalls[i].t_end, from, a, s_loc, t_loc))
					if (s_loc < s) {
						s = s_loc;
						t = t_loc;
						id_wall = pwalls[i].id_wall;

						find = true;
					}

			return find;
		}

		bool place_go_draw(float &x, float &y, float speed, float &s_x) {
			if (-EPS <= y - 1 && y - 1 <= EPS)
				y = 0;
			if (-EPS <= x - 1 && x - 1 <= EPS)
				x = 0;

			float x_d = 1 - x;
			if (x_d * speed + y >= 1)
				x_d = (1 - y) / speed;

			if (x_d >= s_x) {
				x += s_x;
				y += s_x * speed;
				s_x = 0;
				return false;
			}

			x += x_d;
			y += x_d * speed;
			s_x -= x_d;
			return true;
		}

		// for draw_line
		struct node_mem {
			std::uint16_t id;
			// 0b - left, 1b - right
			std::uint16_t flag = 0;
		};

		void change_a_mirror(math::Vec2f &a, math::Vec2f v) {
			float tmp_len = len(a) * len(v);
			float tmp_cos = math::dot(a, v) / tmp_len, tmp_sin = math::cross(a, v) / tmp_len;

			v.x = a.x * tmp_cos - a.y * tmp_sin;
			v.y = a.x * tmp_sin + a.y * tmp_cos;

			a.x = v.x * tmp_cos - v.y * tmp_sin;
			a.y = v.x * tmp_sin + v.y * tmp_cos;
		}

		void draw_line(draw::Window &window, Universe &uni, uchar id_world, math::Vec2f from, math::Vec2f a, int x, int y, int size_y) {
			node_mem *mem = new node_mem[uni.worlds[id_world].get_size_tree() + 1];
			uchar i_stack = 0;
			int i = 0;

			float s_all = 0;

			mem[i].id = mem[i].flag = 0;

			World *world = &uni.worlds[id_world];

			while (i > -1) {
				if (mem[i].id & 0x8000) { // array
					float s, t;
					std::uint16_t id_wall;

					if (check_node(world->walls, world->pwalls + (((int)(mem[i].id & 0x7fff)) << 2), from, a, id_wall, s, t)) {
						bool draw = false;

						// add to stack-draw
						s_all += s;
						draw::add(x, i_stack, s_all, t, from.x, from.y, a.x, a.y, id_wall, id_world);
						++i_stack;

						// special properties
						switch (world->walls[id_wall].type) {
						case 0:
							draw = true;
							break;

						case 1:
							from += a * s;
							++i;
							break;

						case 2:
							from += a * s;

							change_a_mirror(a, (math::Vec2f)world->walls[id_wall].a);

							mem[0].id = mem[0].flag = 0;

							i = 1;
							break;
						case 3:
							go_to_portal(uni, *world, world->walls[id_wall], t, from, a);

							if (world->walls[id_wall].draw_type)
								id_world = world->adata[world->walls[id_wall].id_add_data].id_world;
							else
								id_world = world->walls[id_wall].id_world;
							if (id_world >= uni.size_worlds)
								throw "world isn't";
							world = &uni.worlds[id_world];

							mem[0].id = mem[0].flag = 0;
							i = 1;

							break;
						}

						if (draw || i_stack == DEPTH - 1) { // stop
							delete[] mem;
							return;
						}
					}

					--i;
				} else if (!mem[i].flag) { // chose left or right
					Wall &wall = world->walls[world->nodes[mem[i].id].id_wall];
					if (math::cross((math::Vec2f)wall.a, from - (math::Vec2f)wall.from) > 0) {
						mem[i].flag = 1;

						mem[i + 1].id = world->nodes[mem[i].id].left;
						mem[++i].flag = 0;
					} else {
						mem[i].flag = 2;

						mem[i + 1].id = world->nodes[mem[i].id].right;
						mem[++i].flag = 0;
					}
				} else if (mem[i].flag == 1) { // go right last
					mem[i].id = world->nodes[mem[i].id].right;
					mem[i].flag = 0;
				} else { // go left last
					mem[i].id = world->nodes[mem[i].id].left;
					mem[i].flag = 0;
				}
			}

			draw::add_none(x, i_stack, from.x, from.y, a.x, a.y, id_world);

			delete[] mem;
		}

		void go_to_portal(Universe &uni, World &world, Wall &wall, float t, math::Vec2f &from, math::Vec2f &a) {
			if (wall.type != 3) return;

			Wall *sec_wall;
			if (wall.draw_type) {
				if (world.adata[wall.id_add_data].id_wall >= uni.worlds[world.adata[wall.id_add_data].id_world].walls_size)
					throw "wall isn't";
				sec_wall = &uni.worlds[world.adata[wall.id_add_data].id_world].walls[world.adata[wall.id_add_data].id_wall];
			} else {
				if (uni.worlds[wall.id_world].walls_size <= wall.id_wall)
					throw "wall isn't";
				sec_wall = &uni.worlds[wall.id_world].walls[wall.id_wall];
			}

			const float len = math::len((math::Vec2f)wall.a) * math::len((math::Vec2f)sec_wall->a);
			const float cos_a = math::dot((math::Vec2f)wall.a, (math::Vec2f)sec_wall->a) / len, sin_a = math::cross((math::Vec2f)wall.a, (math::Vec2f)sec_wall->a) / len;

			a = { a.x * cos_a - a.y * sin_a, a.x * sin_a + a.y * cos_a };

			from = (math::Vec2f)sec_wall->from + (math::Vec2f)sec_wall->a * t;
		}
	}
}
