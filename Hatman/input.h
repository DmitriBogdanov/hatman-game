#pragma once

#include <SDL.h> // 'SDL_Event' type
#include <unordered_map> // related type
#include "geometry_utils.h" // Vector2d



// # Input #
// - Holds all pressed/released/held keys for a single frame
// - Basically a wrapper for ugly SDL event system
class Input {
public:
	Input() = default;

	void begin_new_frame();

	void event_KeyDown(const SDL_Event &event); // 'key' in SDL refers to keyboard keys
	void event_KeyUp(const SDL_Event &event);
	
	void event_ButtonDown(const SDL_Event &event); // 'button' in SDL refers to mouse buttons
	void event_ButtonUp(const SDL_Event &event);

	// Getters
	bool key_pressed(SDL_Scancode key);
	bool key_held(SDL_Scancode key);
	bool key_released(SDL_Scancode key);

	bool mouse_pressed(Uint8 button);
	bool mouse_held(Uint8 button);
	bool mouse_released(Uint8 button);
	
	Vector2d mousePosition() const; // mouse position during the last frame
	double mouseX() const; 
	double mouseY() const;

private:
	// Note that [] of nonexistant key initializes pair <key, FALSE> in the map (standard C++14)

	// Keyboard
	std::unordered_map<SDL_Scancode, bool> keys_pressed;
	std::unordered_map<SDL_Scancode, bool> keys_released;
	std::unordered_map<SDL_Scancode, bool> keys_held;

	// Mouse
	std::unordered_map<Uint8, bool> buttons_pressed;
	std::unordered_map<Uint8, bool> buttons_released;
	std::unordered_map<Uint8, bool> buttons_held;

	Vector2d mouse_position; // mouse position in natural 640x360 scale
		// (0, 0) if mouse is outside the window
};