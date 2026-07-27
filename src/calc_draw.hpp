#pragma once

#include "engine_structs.hpp"
#include "draw.hpp"
#include "my_math.hpp"

#define MAX_STACK_DRAW 10

namespace pseudo3d_engine {
	namespace calc {
		bool interact_with_wall(Wall &wall, float t_start, float t_end, math::Vec2f from, math::Vec2f a, float &s, float &t);

		bool check_node(Wall *walls, wall_ptr *pwalls, math::Vec2f from, math::Vec2f a, std::uint16_t &id_wall, float &s, float &t);

		void draw_line(draw::Window &window, Universe &uni, unsigned char id_world, math::Vec2f from, math::Vec2f a, int x, int y, int size_y);

		void go_to_portal(Universe &uni, World &world, Wall &wall, float t, math::Vec2f &from, math::Vec2f &a);
	}
}
