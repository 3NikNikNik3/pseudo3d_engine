#include "draw.hpp"

namespace pseudo3d_engine {
	namespace draw {
		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a) {
			sf::Vertex arr[4] = {
				{ { (float)x, (float)y }, { r, g, b, a } },
				{ { x + 1.0f, (float)y }, { r, g, b, a } },
				{ { (float)x, (float)y + size }, {r, g, b, a } },
				{ { x + 1.0f, (float)y + size }, {r, g, b, a } } };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip);
		}
	}
}
