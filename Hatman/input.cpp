#include "input.h"

#include "graphics.h" // to acces rendering scaling when calculating mouse position



// # Input #
void Input::begin_new_frame() { // pressed/released keys matter only for current 1 frame => we clear them each frame
	this->keys_pressed.clear();
	this->keys_released.clear();
	this->buttons_pressed.clear();
	this->buttons_released.clear();
	
	// Get mouse position relative to the window and scale it to the natural 640x360 scale
	int x, y;
	SDL_GetMouseState(&x, &y); // returns (0, 0) if mouse is outside the window
	this->mouse_position.x = x / Graphics::READ->scaling_factor() * Graphics::READ->camera->zoom;
	this->mouse_position.y = y / Graphics::READ->scaling_factor() * Graphics::READ->camera->zoom;
}

void Input::event_KeyDown(const SDL_Event &event) {
	this->keys_pressed[event.key.keysym.scancode] = true;
	this->keys_held[event.key.keysym.scancode] = true;
}
void Input::event_KeyUp(const SDL_Event &event) {
	this->keys_released[event.key.keysym.scancode] = true;
	this->keys_held[event.key.keysym.scancode] = false;
}

void Input::event_ButtonDown(const SDL_Event &event) {
	this->buttons_pressed[event.button.button] = true;
	this->buttons_held[event.button.button] = true;
}
void Input::event_ButtonUp(const SDL_Event &event) {
	this->buttons_released[event.button.button] = true;
	this->buttons_held[event.button.button] = false;
}

bool Input::key_pressed(const SDL_Scancode key) {
	return this->keys_pressed[key];
}
bool Input::key_released(const SDL_Scancode key) {
	return this->keys_released[key];
}
bool Input::key_held(const SDL_Scancode key) {
	return this->keys_held[key];
}

bool Input::mouse_pressed(Uint8 button) {
	return this->buttons_pressed[button];
}
bool Input::mouse_released(Uint8 button) {
	return this->buttons_released[button];
}
bool Input::mouse_held(Uint8 button) {
	return this->buttons_held[button];
}

Vector2d Input::mousePosition() const {
	return this->mouse_position;
}
double Input::mouseX() const {
	return this->mouse_position.x;
}
double Input::mouseY() const {
	return this->mouse_position.y;
}
