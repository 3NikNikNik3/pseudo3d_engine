#pragma once

#include <SFML/Graphics.hpp>

#define uchar unsigned char

namespace pseudo3d_engine {
	namespace draw {
		struct Window {
			sf::RenderWindow *window;
		};

		struct Image {
			sf::Texture texture;
			int size;
			char *path;

			bool load(const char *path);

			void unload();
		};

		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a);

		void draw_image(Window &win, int x, int y, int size, Image &img, float t);
	}
}
