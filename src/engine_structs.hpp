#pragma once

#include "my_math.hpp"

namespace pseudo3d_engine {
	struct Player {
		math::Vec2f pos;
		float a;
	};

	struct Wall {
		math::Vec2i from, to;

		// 0 - no transparent, 1 - transparent, 2 - portal, 3 - mirror
		uchar type: 4;
		// 0 - nothing, 1 - color, 2 - texture, 3 - animation texture, 4 - animation generating
		uchar draw_type: 4;

		union { // addication data
			// color (*:1)
			struct { uchar r, g, b, a; };

			// (*:2)
			unsigned int id_texture;

			// (*:3)
			unsigned int id_anim_texture;

			// (*:4)
			unsigned int id_anim_gen;

			// if need more memory (2:1|2|3|4)
			unsigned int id_add;

			// for portal (2:0)
			struct { uchar id_word; unsigned int id_wall; };
		};
	};

	//! to binery
	struct World {
		unsigned int size_real, size_now;

		Wall *arr;

		World(unsigned int size);

		~World();

		void resize(unsigned int size);
	};

	struct Universe {
		World *worlds;
		uchar size_worlds;

		Universe();

		~Universe();

		void add_world(unsigned int size);
	};
}
