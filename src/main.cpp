#include <iostream>

#include <SFML/Window.hpp>

#include "engine_functions.hpp"
#include "engine_phys.hpp"

using namespace pseudo3d_engine;

int main() {
	/*// debug save/load
	Universe q;
	load_universe("../data/test.map", q);

	save_universe("../data/test_.map", q);
	save_universe("../data/test_.mapb", q);

	Universe w;
	load_universe("../data/test_.mapb", w);

	save_universe("../data/test_b.map", w);

	return 0;*/

	sf::RenderWindow window(sf::VideoMode({800, 600}), "Test pseudo 3D engine", sf::Style::Default);

	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)window.getSize().x, (float)window.getSize().y})));

	draw::Window win(&window);
	MovingObject player({0, 0}, 1.57, 0);

	Universe uni;
	if (!load_universe("../data/test.map", uni)) {
		window.close();
		return 1;
	}

	sf::Clock clock;

	while (window.isOpen()) {
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
			player.a += 0.001;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
			player.a -= 0.001;
		move(uni, player, math::rotation(math::norm(delta), -player.a) / 1000);

		window.clear({0, 0, 0});

		draw_player_see(win, uni, player, {0, 0}, {(int)window.getSize().x, (int)window.getSize().y}, 1);

		window.display();

		if (false) { // FPS
			std::cout << 1 / clock.restart().asSeconds() << std::endl;
		}
	}

	return 0;
}
