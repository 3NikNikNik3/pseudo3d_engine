#include "engine_structs.hpp"

#include <cstdlib>

namespace pseudo3d_engine {
	// World
	World::World(unsigned short size) : walls_size_real(size), walls_size_now(0) {
		walls = (Wall*)std::malloc(sizeof(Wall) * size);
		if (walls == nullptr)
			throw "little memory";
	}

	World::~World() {
		std::free(walls);
	}

	void World::resize(unsigned short size) {
		if (size <= walls_size_real)
			return;
		void *ptr = std::realloc(walls, sizeof(Wall) * size);
		if (ptr == nullptr)
			throw "little memory";
		walls = (Wall*)ptr;

		walls_size_real = size;
	}

	// Universe
	Universe::Universe() : worlds(nullptr), size_worlds(0) { }

	Universe::~Universe() {
		delete[] worlds;
	}

	void Universe::add_world(unsigned short size) {
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
