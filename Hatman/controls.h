#pragma once

#include <SDL_events.h> // 'SDL_Scancode' type
#include <SFML/Graphics.hpp> /// Perhaps unnecessary



class Controls {
public:
	Controls();

	static const Controls* READ;
	static Controls* ACCESS;

	// GUI controls
	sf::Mouse::Button LMB;
	sf::Keyboard::Key ESC;
	sf::Keyboard::Key F3;

	// Game controls
	sf::Keyboard::Key LEFT;
	sf::Keyboard::Key RIGHT;
	sf::Keyboard::Key UP;
	sf::Keyboard::Key DOWN;

	sf::Mouse::Button CHAIN;
	sf::Keyboard::Key SHIFT;
	sf::Keyboard::Key SKILL;

	sf::Keyboard::Key ZOOMOUT;

	sf::Keyboard::Key JUMP;
	sf::Keyboard::Key USE;

	sf::Keyboard::Key INVENTORY;
};


std::string toString(sf::Mouse::Button button);
std::string toString(sf::Keyboard::Key key);