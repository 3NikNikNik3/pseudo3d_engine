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

	void move(Universe &uni, uchar id_world, MovingObject &obj, math::Vec2f delta, int count) {
		// delta == {0,0}
		if (count == 5 || (-EPS <= delta.x && delta.x <= EPS && -EPS <= delta.y && delta.y <= EPS))
			return;

		std::uint16_t id_wall;
		float s;

		if (what_see(uni, id_world, obj.pos, math::norm(delta), &id_wall, &s, nullptr)) {
			float delta_len = math::len(delta);

			if (s > delta_len && s > SIZE_WALL)
				obj.pos += delta;
			else {
				//! portal: teleport

				if (s > SIZE_WALL) {
					obj.pos += delta * ((s - SIZE_WALL) / delta_len);

					delta *= (delta_len - s + SIZE_WALL) / delta_len;
				}

				math::Vec2f a = (math::Vec2f)uni.worlds[id_world].walls[id_wall].a;

				a /= math::len(a);

				move(uni, id_world, obj, a * (math::dot(a, delta)), count + 1);
			}
		} else
			obj.pos += delta;
	}
}
