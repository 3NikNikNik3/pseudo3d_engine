#include "engine_phys.hpp"

#include "my_math.hpp"
#include "calc_draw.hpp"

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

	void move(Universe &uni, MovingObject &obj, math::Vec2f delta, int count) {
		// delta == {0,0}
		if (count == 5 || (-EPS <= delta.x && delta.x <= EPS && -EPS <= delta.y && delta.y <= EPS))
			return;

		std::uint16_t id_wall;
		float s, t;

		if (what_see(uni, obj.id_world, obj.pos, math::norm(delta), &id_wall, &s, &t)) {
			float delta_len = math::len(delta);

			Wall &wall = uni.worlds[obj.id_world].walls[id_wall];

			// teleport
			if (wall.type == 3) {
				if (s > delta_len)
					obj.pos += delta;
				else {
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

					obj.pos += delta * delta_len;
				}

				return;
			}

			// pass
			if (wall.phys_pass) {
				obj.pos += delta;
				return;
			}

			// no pass
			if (s > delta_len && s > SIZE_WALL)
				obj.pos += delta;
			else {
				if (s > SIZE_WALL) {
					obj.pos += delta * ((s - SIZE_WALL) / delta_len);

					delta *= (delta_len - s + SIZE_WALL) / delta_len;
				}

				math::Vec2f a = (math::Vec2f)wall.a;

				a /= math::len(a);

				move(uni, obj, a * (math::dot(a, delta)), count + 1);
			}
		} else
			obj.pos += delta;
	}
}
