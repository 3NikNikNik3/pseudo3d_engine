#include <SFML/Window.hpp>

#include <iostream>

#include "engine_functions.hpp"

using namespace pseudo3d_engine;

int main() {
	sf::RenderWindow window(sf::VideoMode({800, 600}), "Test pseudo 3D engine", sf::Style::Default);

	window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {(float)window.getSize().x, (float)window.getSize().y})));

	draw::Window win(&window);
	Player player({0, 0}, 0);
	Universe uni;

	uni.add_world(1);

	uni.worlds[0].walls[0].from = {1, 1};
	uni.worlds[0].walls[0].a = {1, -2};
	uni.worlds[0].walls[0].type = 0;
	uni.worlds[0].walls[0].draw_type = 1;
	uni.worlds[0].walls[0].r = uni.worlds[0].walls[0].g = uni.worlds[0].walls[0].b = 255;
	uni.worlds[0].walls_size_now = 1;

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
		player.pos += math::rotation(math::norm(move), player.a) / 1000;

		window.clear({0, 0, 0});

		draw_player_see(win, uni, 0, player, {0, 0}, {(int)window.getSize().x, (int)window.getSize().y}, 0.5);

		window.display();
	}

	return 0;
}
