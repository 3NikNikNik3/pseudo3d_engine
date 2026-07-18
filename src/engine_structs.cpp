#include "engine_structs.hpp"

#include <cstdlib>

namespace pseudo3d_engine {
	// World
	World::World(std::uint16_t size) : walls_size(size), node_size(0), wall_ptr_size(0), nodes(nullptr), pwalls(nullptr) {
		walls = (Wall*)std::malloc(sizeof(Wall) * size);
		if (walls == nullptr)
			throw "little memory";
	}

	World::~World() {
		std::free(walls);
		std::free(nodes);
		std::free(pwalls);
	}

	void World::resize_walls(std::uint16_t size) {
		if (size <= walls_size)
			return;
		void *ptr = std::realloc(walls, sizeof(Wall) * size);
		if (ptr == nullptr)
			throw "little memory";
		walls = (Wall*)ptr;

		walls_size = size;
	}

	void World::resize_nodes(std::uint16_t size) {
		if (size < node_size) return;

		if (nodes == nullptr)
			nodes = (node*)std::malloc(sizeof(node) * size);
		else {
			void *ptr = std::realloc(nodes, sizeof(node) * size);
			if (ptr == nullptr)
				throw "little memory";
			nodes = (node*)ptr;
		}

		node_size = size;
	}

	void World::resize_pwalls(std::uint16_t size) {
		if (size < wall_ptr_size) return;

		if (pwalls == nullptr)
			pwalls = (wall_ptr*)std::malloc((sizeof(wall_ptr) * size) << 2);
		else {
			void *ptr = std::realloc(pwalls, (sizeof(wall_ptr) * size) << 2);
			if (ptr == nullptr)
				throw "little memory";
			pwalls = (wall_ptr*)ptr;
		}
	}

	// Universe
	Universe::Universe() : worlds(nullptr), size_worlds(0) { }

	Universe::~Universe() {
		delete[] worlds;
	}

	void Universe::add_world(std::uint16_t size) {
		if (worlds == nullptr) {
			size_worlds = 1;
			worlds = new World[1]{size};
		} else {
			void *ptr = std::realloc(worlds, sizeof(World) * (size_worlds + 1));
			if (ptr == nullptr)
				throw "little memory";
			worlds = (World*)ptr;

			worlds[size_worlds++] = World(size);
		}
	}
}
