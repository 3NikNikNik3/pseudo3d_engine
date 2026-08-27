#pragma once

#include <cstdint>

#include "my_math.hpp"
#include "draw.hpp"

namespace pseudo3d_engine {
	struct MovingObject {
		math::Vec2f pos;
		float a;
		// id_node_now is cache for phys::move and entity::draw
		// id_node_now: 0x8000 - left or right, 0x7fff - real node's id
		// id_node_now and len_node_now is 0 by default!
		std::uint16_t id_node_now, len_node_now;
		uchar id_world;
	};

	struct Wall {
		math::Vec2i from, a;

		// 0 - no transparent, 1 - transparent, 2 - mirror, 3 - portal
		uchar type: 3;
		// 0 - nothing, 1 - color, 2 - texture, 3 - animation texture, 4 - animation generating
		uchar draw_type: 4;
		uchar phys_pass: 1;

		union { // addication data
			// color (*:1)
			struct { uchar r, g, b, alpha; };

			// (*:2)
			unsigned int id_texture;

			// (*:3)
			unsigned int id_anim_texture;

			// (*:4)
			unsigned int id_anim_gen;

			// if need more memory (3:1|2|3|4)
			unsigned int id_add_data;

			// for portal (3:0)
			struct { std::uint16_t id_wall; uchar id_world; };
		};
	};

	struct add_data { // sizeof = 8
		union {
			struct { // for wall
				union { // for draw
					struct { uchar r, g, b, alpha; };

					unsigned int id_texture;

					unsigned int id_anim_texture;

					unsigned int id_anim_gen;
				};

				// for portal
				struct { std::uint16_t id_wall; uchar id_world; };
			};
		};
	};

	struct Place {
		// 0 - floor, 1 - sky
		uchar draw_type: 1;
		// 0 - color, 1 - image, 2 - anim gen
		uchar type: 7;

		union {
			// (*:0)
			struct { uchar r, g, b; };

			// (*:1)
			unsigned int id_texture;

			// (*:2)
			unsigned int id_anim_gen;
		};
	};

	struct node {
		std::uint16_t id_wall;
		// 0b: 0 - node, 1 - array; 1-15b: id
		std::uint16_t left, right, back;
	};

	struct wall_ptr {
		float t_start, t_end;
		std::uint16_t id_wall;
	};

	struct World {
		std::uint16_t walls_size = 0, node_size = 0, wall_ptr_size = 0, tree_size = 0;
		std::uint16_t add_data_size = 0;

		Place up, down;

		Wall *walls = nullptr;
		node *nodes = nullptr;
		wall_ptr *pwalls = nullptr; // end is t_end == 0
		add_data *adata = nullptr;

		World();

		~World();

		// when you use resize_*, you must put data on all new size
		void resize_walls(std::uint16_t size);

		void resize_nodes(std::uint16_t size);

		void resize_pwalls(std::uint16_t size);

		void resize_adata(std::uint16_t size);

		std::uint16_t get_size_tree();

		void make_tree();
	};

	struct Universe {
		World *worlds = nullptr;
		draw::Image *images = nullptr;

		unsigned int size_image = 0;
		uchar size_worlds = 0;

		Universe();

		~Universe();

		void set_worlds(uchar size);

		void resize_images(unsigned int size);
	};

	// for const (on run) settings
	struct Settings {
		uchar depth_draw;
	};

	Settings& get_settings();
}
