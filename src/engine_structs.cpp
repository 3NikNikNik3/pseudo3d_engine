#include "engine_structs.hpp"

#include <cstdlib>

namespace pseudo3d_engine {
	// World
	World::World(unsigned int size) : size_real(size), size_now(0) {
		arr = (Wall*)std::malloc(sizeof(Wall) * size);
		if (arr == nullptr)
			throw "little memory";
	}

	World::~World() {
		std::free(arr);
	}

	void World::resize(unsigned int size) {
		if (size <= size_real)
			return;
		void *ptr = std::realloc(arr, sizeof(Wall) * size);
		if (ptr == nullptr)
			throw "little memory";
		arr = (Wall*)ptr;

		size_real = size;
	}

	// Universe
	Universe::Universe() : worlds(nullptr), size_worlds(0) { }

	Universe::~Universe() {
		delete[] worlds;
	}

	void Universe::add_world(unsigned int size) {
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
