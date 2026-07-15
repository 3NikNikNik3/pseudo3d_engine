#pragma once

#include "engine_structs.hpp"
#include "draw.hpp"
#include "my_math.hpp"

namespace pseudo3d_engine {
	namespace calc {
		void draw_line(draw::Window &window, Universe &uni, math::Vec2f from, math::Vec2f a, int x);
	}
}
