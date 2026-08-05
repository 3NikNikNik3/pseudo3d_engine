#include "draw.hpp"

#define vertex_line_arr sf::Vertex arr[4] = { \
			{ { (float)x, (float)y } }, \
			{ { x + 1.0f, (float)y } }, \
			{ { (float)x, (float)y + size } }, \
			{ { x + 1.0f, (float)y + size } } };


namespace pseudo3d_engine {
	namespace draw {
		// Image
		bool Image::load(const char *path) {
			for (size = 1; path[size]; ++size);

			this->path = new char[size + 1];
			for (int i = 0; i <= size; ++i)
				this->path[i] = path[i];

			sf::Image image;
			if (!image.loadFromFile(path))
				return false;
			return texture.loadFromImage(image);
		}

		void Image::unload() {
			delete[] path;
		}

		int Image::get_x() {
			return texture.getSize().x;
		}

		int Image::get_y() {
			return texture.getSize().y;
		}

		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a) {
			vertex_line_arr;

			arr[0].color = arr[1].color = arr[2].color = arr[3].color = { r, g, b, a };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip);
		}

		void draw_image(Window &win, int x, int y, int size, Image &img, float t) {
			vertex_line_arr;

			sf::Vector2f pos = { img.texture.getSize().x * t, (float)img.texture.getSize().y };

			arr[0].texCoords = { pos.x - 1, 0 };
			arr[1].texCoords = { pos.x + 1, 0 };
			arr[2].texCoords = { pos.x - 1, pos.y };
			arr[3].texCoords = { pos.x + 1, pos.y };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip, &img.texture);
		}

		/*void draw_part_image(Window &win, int x, int y, int size, Image &img, float from_img_x, float from_img_y, float to_img_x, float to_img_y) {
			vertex_line_arr;

			arr[0].texCoords = { from_img_x, from_img_y };
			arr[1].texCoords = { to_img_x, from_img_y };
			arr[2].texCoords = { from_img_x, to_img_y };
			arr[3].texCoords = { to_img_x, to_img_y };

			win.window->draw(arr, 4, sf::PrimitiveType::TriangleStrip, &img.texture);
		}*/
	}
}
