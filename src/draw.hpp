#pragma once

#include <SFML/Graphics.hpp>

#include <GL/glew.h>

#define uchar unsigned char

namespace pseudo3d_engine {
	struct Universe;

	namespace draw {
		struct Window {
			sf::RenderWindow *window;
		};

		struct Image {
			GLuint texture;
			int size, x, y;
			char *path;

			bool load(const char *path);

			void unload();

			int get_x();

			int get_y();
		};

		bool init();

		void init_buff(unsigned int size_new);

		void deinit();

		void add(unsigned int x, uchar depth, float s, float t, float from_x, float from_y, float a_x, float a_y, std::uint16_t id_wall, uchar id_world);

		void add_none(unsigned int x, uchar depth, float from_x, float from_y, float a_x, float a_y, uchar id_world);

		void draw(Window &win, Universe &uni, int x, int y, int size_x, int size_y);
	}
}
