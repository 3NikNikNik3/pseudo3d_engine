#pragma once

#include <SFML/Graphics.hpp>

#define uchar unsigned char

#define DEPTH 16

namespace pseudo3d_engine {
	struct Universe;

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

			int get_x();

			int get_y();
		};

		void init(unsigned int size_new);

		void deinit();

		void add(unsigned int x, uchar depth, float s, float t, float from_x, float from_y, float a_x, float a_y, std::uint16_t id_wall, uchar id_world);

		void add_none(unsigned int x, uchar depth, float from_x, float from_y, float a_x, float a_y, uchar id_world);

		void draw(Window &win, Universe &uni, int x, int y, int size_x, int size_y);

		void draw_line(Window &win, int x, int y, int size, uchar r, uchar g, uchar b, uchar a);

		void draw_image(Window &win, int x, int y, int size, Image &img, float t);

		// void draw_part_image(Window &win, int x, int y, int size, Image &img, float from_img_x, float from_img_y, float to_img_x, float to_img_y);
	}
}
