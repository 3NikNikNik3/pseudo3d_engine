#pragma once

#include <cstdint>

#include "my_math.hpp"

namespace pseudo3d_engine {
	struct Player {
		math::Vec2f pos;
		float a;
	};

	struct Wall {
		math::Vec2i from;
		math::Vec2f a;

		// 0 - no transparent, 1 - transparent, 2 - portal, 3 - mirror
		uchar type: 4;
		// 0 - nothing, 1 - color, 2 - texture, 3 - animation texture, 4 - animation generating
		uchar draw_type: 4;

		union { // addication data
			// color (*:1)
			struct { uchar r, g, b, alpha; };

			// (*:2)
			unsigned int id_texture;

			// (*:3)
			unsigned int id_anim_texture;

			// (*:4)
			unsigned int id_anim_gen;

			// if need more memory (2:1|2|3|4)
			unsigned int id_add_data;

			// for portal (2:0)
			struct { std::uint16_t id_wall; uchar id_world; };
		};
	};

	struct node {
		std::uint16_t id_wall;
		// 0b: 0 - node, 1 - array; 1-15b: id
		std::uint16_t left, right;
	};

	struct wall_ptr {
		float t_start, t_end;
		std::uint16_t id_wall;
	};

	struct World {
		std::uint16_t walls_size, node_size, wall_ptr_size, tree_size = 0;

		Wall *walls;
		node *nodes;
		wall_ptr *pwalls; // end is t_end == 0

		World(std::uint16_t size);

		~World();

		// when you use resize_*, you must put data on all new size
		void resize_walls(std::uint16_t size);

		void resize_nodes(std::uint16_t size);

		void resize_pwalls(std::uint16_t size);

		std::uint16_t get_size_tree();
	};

	struct Universe {
		World *worlds;
		uchar size_worlds;

		Universe();

		~Universe();

		void add_world(std::uint16_t size);
	};
}
