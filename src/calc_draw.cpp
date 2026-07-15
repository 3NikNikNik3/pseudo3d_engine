#include "calc_draw.hpp"

namespace pseudo3d_engine {
	namespace calc {
		void draw_line(draw::Window &window, Universe &uni, math::Vec2f from, math::Vec2f a, int x) {
			draw::draw_line(window, x, 10, 200, abs(255 * sin(x / 100.0)), 0, 0);
		}
	}
}
