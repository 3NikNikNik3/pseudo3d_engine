#include "engine_functions.hpp"

#include "calc_draw.hpp"

using namespace pseudo3d_engine::math;

namespace pseudo3d_engine {
	void draw_player_see(draw::Window &window, Universe &uni, int id_world, Player &player, Vec2i from, Vec2i to, float angl_see) {
		const float a_shift = angl_see / (to.x - from.x), start_a = player.a - angl_see / 2;

		for (int i = 0; i <= to.x - from.x; ++i)
			calc::draw_line(window, uni, id_world, player.pos, Vec2f::from_a(start_a + a_shift * i), i, from.y, to.y - from.y);
	}
}
