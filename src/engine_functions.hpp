#pragma once

#include "engine_structs.hpp"
#include "draw.hpp"

namespace pseudo3d_engine {
	void draw_player_see(draw::Window &window, Universe &uni, MovingObject &player, math::Vec2i from, math::Vec2i to, float angl_see);

	bool load_universe_map(const char *path, Universe &ans);

	void save_universe_map(const char *path, const Universe &uni);

	bool load_universe_mapb(const char *path, Universe &ans);

	void save_universe_mapb(const char *path, const Universe &uni);

	bool load_universe_mapo(const char *path, Universe &ans);

	void save_universe_mapo(const char *path, const Universe &uni);

	bool load_universe(const char *path, Universe &uni, bool make_tree = true);

	void save_universe(const char *path, const Universe &uni);

	void init();

	void deinit();
}
