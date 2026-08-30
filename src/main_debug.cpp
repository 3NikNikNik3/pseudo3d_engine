#include <iostream>

#include <SFML/Window.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine_functions.hpp"
#include "engine_phys.hpp"

using namespace pseudo3d_engine;

int main(int argc, char *argv[]) {
	/*// debug save/load
	Universe q;
	std::cout << load_universe("../data/test.map", q) << std::endl;

	save_universe("../data/test_.map", q);
	save_universe("../data/test_.mapb", q);

	Universe w;
	std::cout << load_universe("../data/test_.mapb", w) << std::endl;

	save_universe("../data/test_b.map", w);

	return 0;*/

	const char *map_path = "../data/test.map";
	if (argc == 2)
		map_path = argv[1];

	sf::RenderWindow window(sf::VideoMode({800, 600}), "Test pseudo 3D engine", sf::Style::Default);

	if (!init(nullptr)) {
		deinit();
		return 1;
	}

	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)window.getSize().x, (float)window.getSize().y})));

	MovingObject player({0, 0}, 1.57, 0, 0, 0);

	Universe uni;
	if (!load_universe(map_path, uni)) {
		window.close();
		deinit();
		return 1;
	}

	sf::Clock clock;

	while (window.isOpen()) {
		const float d = clock.restart().asSeconds();

		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
				window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)resized->size.x, (float)resized->size.y})));
			}
		}

		math::Vec2f delta = {0, 0};
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
			delta.x += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
			delta.x -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
			delta.y += 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
			delta.y -= 1;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
			player.a += math::pi / 4 * d;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
			player.a -= math::pi / 4 * d;
		move(uni, player, math::rotation(math::norm(delta), player.a) * d);

		window.clear({0, 0, 0});

		draw_player_see(window.getSize().x, window.getSize().y, uni, player, {0, 0}, {(int)window.getSize().x, (int)window.getSize().y}, 1);

		window.display();

		if (false) { // FPS
			std::cout << 1 / d << std::endl;
		}
	}

	deinit();

	return 0;
}
