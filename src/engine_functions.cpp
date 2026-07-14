#include "engine_functions.hpp"

using namespace pseudo3d_engine::math;

namespace pseudo3d_engine {
	void draw_line(draw::Window &window, Universe &uni, Vec2f from, Vec2f a) {
		// nothing :)
	}

	void draw_player_see(draw::Window &window, Universe &uni, Player &player, Vec2i from, Vec2i to, float angl_see) {
		const float a_shift = angl_see / (from.y - to.y), start_a = player.a - angl_see / 2;

		for (int i = 0; i < from.y - to.y; ++i)
			draw_line(window, uni, player.pos, Vec2f_from_a(start_a + a_shift * i));
	}
}
