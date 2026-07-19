#pragma once

#include "engine_structs.hpp"
#include "draw.hpp"
#include "my_math.hpp"

namespace pseudo3d_engine {
	namespace calc {
		bool interact_with_wall(Wall &wall, float t_start, float t_end, math::Vec2f from, math::Vec2f a, float &s, float &t);

		void draw_line(draw::Window &window, Universe &uni, unsigned char id_world, math::Vec2f from, math::Vec2f a, int x, int y, int size_y);
	}
}
