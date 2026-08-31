#include "engine_phys.hpp"

#include "calc_draw.hpp"
#include "engine_functions.hpp"

namespace pseudo3d_engine {
	bool what_see_real(World &world, math::Vec2f from, math::Vec2f a, std::uint16_t id_node, std::uint16_t *id_wall, float *s, float *t) {
		if (id_node & 0x8000) {
			float s_, t_;
			std::uint16_t id_wall_;

			if (calc::check_node(world.walls, world.pwalls + (((int)(id_node & 0x7fff)) << 2), from, a, id_wall_, s_, t_)) {
				if (id_wall != nullptr)
					*id_wall = id_wall_;
				if (s != nullptr)
					*s = s_;
				if (t != nullptr)
					*t = t_;

				return true;
			}
			return false;
		}

		Wall &wall = world.walls[world.nodes[id_node].id_wall];
		if (math::cross((math::Vec2f)wall.a, from - (math::Vec2f)wall.from) > 0) {
			if (what_see_real(world, from, a, world.nodes[id_node].left, id_wall, s, t))
				return true;

			return what_see_real(world, from, a, world.nodes[id_node].right, id_wall, s, t);
		} else {
			if (what_see_real(world, from, a, world.nodes[id_node].right, id_wall, s, t))
				return true;

			return what_see_real(world, from, a, world.nodes[id_node].left, id_wall, s, t);
		}
	}

	bool what_see(Universe &uni, uchar id_world, math::Vec2f from, math::Vec2f a, std::uint16_t *id_wall, float *s, float *t) {
		return what_see_real(uni.worlds[id_world], from, a, 0, id_wall, s, t);
	}

	std::uint16_t get_lca(node *nodes, std::uint16_t x, std::uint16_t len_x, std::uint16_t y, std::uint16_t len_y) {
		if (x == y) {
			if (x & 0x8000)
				return nodes[x & 0x7fff].right;
			return nodes[x].left;
		}

		x &= 0x7fff;
		y &= 0x7fff;
		--len_x;
		--len_y;

		while (len_x > len_y) {
			x = nodes[x].back;
			--len_x;
		}

		while (len_y > len_x) {
			y = nodes[y].back;
			--len_y;
		}

		while (x != y) {
			x = nodes[x].back;
			y = nodes[y].back;

			--len_x;
			--len_y;
		}

		return x;
	}

	bool move(Universe &uni, MovingObject &obj, math::Vec2f delta, int count) {
		// delta == {0,0}
		if (count == 5 || (-EPS <= delta.x && delta.x <= EPS && -EPS <= delta.y && delta.y <= EPS))
			return false;

		std::uint16_t id_node_will, len_node_will;
		get_node(uni.worlds[obj.id_world], obj.pos + delta, id_node_will, len_node_will);

		if (!obj.len_node_now)
			get_node(uni.worlds[obj.id_world], obj.pos, obj.id_node_now, obj.len_node_now);

		std::uint16_t id_wall;
		float s, t;

		if (what_see_real(uni.worlds[obj.id_world], obj.pos, math::norm(delta), get_lca(uni.worlds[obj.id_world].nodes, obj.id_node_now, obj.len_node_now, id_node_will, len_node_will), &id_wall, &s, &t)) {
			float delta_len = math::len(delta);

			Wall &wall = uni.worlds[obj.id_world].walls[id_wall];

			// teleport
			if (wall.type == 3) {
				if (s > delta_len) {
					obj.pos += delta;

					if (obj.id_node_now == id_node_will)
						return false;

					obj.id_node_now = id_node_will;
					obj.len_node_now = len_node_will;
					return true;
				} else {
					delta /= delta_len;
					delta_len -= s;

					calc::go_to_portal(uni, uni.worlds[obj.id_world], uni.worlds[obj.id_world].walls[id_wall], t, obj.pos, delta);

					Wall *sec_wall;
					if (wall.draw_type) {
						add_data &adata = uni.worlds[obj.id_world].adata[wall.id_add_data];
						sec_wall = &uni.worlds[adata.id_world].walls[adata.id_wall];

						obj.id_world = adata.id_world;
					} else {
						sec_wall = &uni.worlds[wall.id_world].walls[wall.id_wall];

						obj.id_world = wall.id_world;
					}
					obj.a += math::get_angle((math::Vec2f)wall.a, (math::Vec2f)sec_wall->a);

					obj.len_node_now = 0;

					move(uni, obj, delta * delta_len);
					return true;
				}
			}

			// pass
			if (wall.phys_pass) {
				obj.pos += delta;

				if (obj.id_node_now == id_node_will)
					return false;

				obj.id_node_now = id_node_will;
				obj.len_node_now = len_node_will;
				return true;
			}

			// no pass
			if (s > delta_len && s > SIZE_WALL) {
				obj.pos += delta;

				if (obj.id_node_now == id_node_will)
					return false;

				obj.id_node_now = id_node_will;
				obj.len_node_now = len_node_will;
				return true;
			} else {
				if (s > SIZE_WALL) {
					obj.pos += delta * ((s - SIZE_WALL) / delta_len);

					delta *= (delta_len - s + SIZE_WALL) / delta_len;
				}

				math::Vec2f a = (math::Vec2f)wall.a;

				a /= math::len(a);

				obj.len_node_now = 0;

				return move(uni, obj, a * (math::dot(a, delta)), count + 1);
			}
		} else {
			obj.pos += delta;

			if (obj.id_node_now == id_node_will)
				return false;

			obj.id_node_now = id_node_will;
			obj.len_node_now = len_node_will;
			return true;
		}
	}
}
