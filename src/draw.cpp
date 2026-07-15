#include "draw.hpp"

namespace pseudo3d_engine {
	namespace draw {
		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b) {
			static sf::RectangleShape rect({1, 1});

			rect.setSize({1, (float)size});
			rect.setPosition({(float)x, (float)y});
			rect.setFillColor({r, g, b});

			win.window->draw(rect);
		}
	}
}
