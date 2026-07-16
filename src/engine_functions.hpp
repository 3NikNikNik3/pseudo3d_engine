#pragma once

#include "engine_structs.hpp"
#include "draw.hpp"

namespace pseudo3d_engine {
	void draw_player_see(draw::Window &window, Universe &uni, int id_world, Player &player, math::Vec2i from, math::Vec2i to, float angl_see);
}
