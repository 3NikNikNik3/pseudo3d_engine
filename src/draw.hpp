#pragma once

#include <SFML/Graphics.hpp>

#define uchar unsigned char

namespace pseudo3d_engine {
	namespace draw {
		struct Window {
			sf::RenderWindow *window;
		};

		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a);
	}
}
