#include <SFML/Window.hpp>

#include "engine_functions.hpp"

using namespace pseudo3d_engine;

int main() {
	sf::RenderWindow window(sf::VideoMode({800, 600}), "Test pseudo 3D engine", sf::Style::Default);

	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)window.getSize().x, (float)window.getSize().y})));

	draw::Window win(&window);
	Player player({0, 0}, 1.57);
	Universe uni;

	uni.add_world(8);

	uni.worlds[0].walls[0].from = {-1, 1};
	uni.worlds[0].walls[0].a = {0, 3};
	uni.worlds[0].walls[1].from = {-1, 4};
	uni.worlds[0].walls[1].a = {3, 0};
	uni.worlds[0].walls[2].from = {2, 4};
	uni.worlds[0].walls[2].a = {0, -3};
	uni.worlds[0].walls[3].from = {2, 1};
	uni.worlds[0].walls[3].a = {-1, 0};
	uni.worlds[0].walls[4].from = {1, 1};
	uni.worlds[0].walls[4].a = {0, 2};
	uni.worlds[0].walls[5].from = {1, 3};
	uni.worlds[0].walls[5].a = {-1, 0};
	uni.worlds[0].walls[6].from = {0, 3};
	uni.worlds[0].walls[6].a = {0, -2};
	uni.worlds[0].walls[7].from = {0, 1};
	uni.worlds[0].walls[7].a = {-1, 0};

	for (int i = 0; i < 8; ++i) {
		uni.worlds[0].walls[i].type = 0;
		uni.worlds[0].walls[i].draw_type = 1;
		uni.worlds[0].walls[i].r = uni.worlds[0].walls[0].g = uni.worlds[0].walls[0].b = uni.worlds[0].walls[i].alpha = 255;
	}

	uni.worlds[0].walls[4].b = 255;
	uni.worlds[0].walls[6].g = 255;

	uni.worlds[0].resize_nodes(2);

	uni.worlds[0].nodes[0].id_wall = 4;
	uni.worlds[0].nodes[0].left = 1;
	uni.worlds[0].nodes[0].right = (1 << 15);
	uni.worlds[0].nodes[1].id_wall = 6;
	uni.worlds[0].nodes[1].right = (1 << 15) | 1;
	uni.worlds[0].nodes[1].left = (1 << 15) | 2;

	uni.worlds[0].resize_pwalls(3);

uni.worlds[0].pwalls[0].id_wall = 3;
uni.worlds[0].pwalls[0].t_start = 0;
uni.worlds[0].pwalls[0].t_end = 1;
uni.worlds[0].pwalls[1].id_wall = 2;
uni.worlds[0].pwalls[1].t_start = 0;
uni.worlds[0].pwalls[1].t_end = 1;
uni.worlds[0].pwalls[2].id_wall = 1;
uni.worlds[0].pwalls[2].t_start = 2.0/3;
uni.worlds[0].pwalls[2].t_end = 1;
uni.worlds[0].pwalls[3].id_wall = 4;
uni.worlds[0].pwalls[3].t_start = 0;
uni.worlds[0].pwalls[3].t_end = 1;
uni.worlds[0].pwalls[4].id_wall = 0;
uni.worlds[0].pwalls[4].t_start = 0;
uni.worlds[0].pwalls[4].t_end = 1;
uni.worlds[0].pwalls[5].id_wall = 1;
uni.worlds[0].pwalls[5].t_start = 0;
uni.worlds[0].pwalls[5].t_end = 1.0/3;
uni.worlds[0].pwalls[6].id_wall = 7;
uni.worlds[0].pwalls[6].t_start = 0;
uni.worlds[0].pwalls[6].t_end = 1;
uni.worlds[0].pwalls[7].id_wall = 6;
uni.worlds[0].pwalls[7].t_start = 0;
uni.worlds[0].pwalls[7].t_end = 1;
uni.worlds[0].pwalls[8].id_wall = 1;
uni.worlds[0].pwalls[8].t_start = 1.0/3;
uni.worlds[0].pwalls[8].t_end = 2.0/3;
uni.worlds[0].pwalls[9].id_wall = 5;
uni.worlds[0].pwalls[9].t_start = 0;
uni.worlds[0].pwalls[9].t_end = 1;
uni.worlds[0].pwalls[10].t_end = 0;

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)resized->size.x, (float)resized->size.y})));
			}
		}

		math::Vec2f move = {0, 0};
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			move.x += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			move.x -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			move.y += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			move.y -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
			player.a += 0.001;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
			player.a -= 0.001;
		player.pos += math::rotation(math::norm(move), -player.a) / 1000;

		window.clear({0, 0, 0});

		draw_player_see(win, uni, 0, player, {0, 0}, {(int)window.getSize().x, (int)window.getSize().y}, 1);

		window.display();
	}

	return 0;
}
