#pragma once

#include <SFML/Graphics.hpp> /// Perhaps system is enough?

#include <unordered_map> // related type
#include "utility/geometry.h" // Vector2d



// # Input #
// - Holds all pressed/released/held keys for a single frame
// - Basically a wrapper for ugly SDL event system
class Input {
public:
	Input() = default;

	void begin_new_frame();

	void event_MouseMove(const sf::Event &event);

	void event_KeyDown(const sf::Event &event); // 'key' in SDL refers to keyboard keys
	void event_KeyUp(const sf::Event &event);
	
	void event_ButtonDown(const sf::Event &event); // 'button' in SDL refers to mouse buttons
	void event_ButtonUp(const sf::Event &event);
	
	// Getters
	bool key_pressed(sf::Keyboard::Key key);
	bool key_held(sf::Keyboard::Key key);
	bool key_released(sf::Keyboard::Key key);

	bool mouse_pressed(sf::Mouse::Button button);
	bool mouse_held(sf::Mouse::Button button);
	bool mouse_released(sf::Mouse::Button button);
	
	Vector2d mousePosition() const; // mouse position during the last frame
	double mouseX() const; 
	double mouseY() const;

private:
	// Note that [] of nonexistant key initializes pair <key, FALSE> in the map (standard C++14)

	// Keyboard
	std::unordered_map<sf::Keyboard::Key, bool> keys_pressed;
	std::unordered_map<sf::Keyboard::Key, bool> keys_released;
	std::unordered_map<sf::Keyboard::Key, bool> keys_held;

	// Mouse
	std::unordered_map<sf::Mouse::Button, bool> buttons_pressed;
	std::unordered_map<sf::Mouse::Button, bool> buttons_released;
	std::unordered_map<sf::Mouse::Button, bool> buttons_held;

	Vector2d mouse_position; // mouse position in natural 640x360 scale
		// (0, 0) if mouse is outside the window
};