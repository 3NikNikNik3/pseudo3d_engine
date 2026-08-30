#pragma once

#include "engine_structs.hpp"

namespace pseudo3d_engine {
	void draw_player_see(int w, int h, Universe &uni, MovingObject &player, math::Vec2i from, math::Vec2i to, float angl_see);

	bool load_universe_map(const char *path, Universe &ans);

	void save_universe_map(const char *path, const Universe &uni);

	bool load_universe_mapb(const char *path, Universe &ans);

	void save_universe_mapb(const char *path, const Universe &uni);

	bool load_universe_mapo(const char *path, Universe &ans);

	void save_universe_mapo(const char *path, const Universe &uni);

	bool load_universe(const char *path, Universe &uni, bool make_tree = true);

	void save_universe(const char *path, const Universe &uni);

	// nullptr, to default
	bool init(const char *path_settings);

	void deinit();

	void save_settings(const char *path);

	// not for lib
	void get_node(World &world, math::Vec2f pos, std::uint16_t &id_node, std::uint16_t &len_node);
}
