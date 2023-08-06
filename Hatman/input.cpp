#include "input.h"

#include "graphics.h" // to acces rendering scaling when calculating mouse position



// # Input #
void Input::begin_new_frame() { // pressed/released keys matter only for current 1 frame => we clear them each frame
	this->keys_pressed.clear();
	this->keys_released.clear();
	this->buttons_pressed.clear();
	this->buttons_released.clear();
}

void Input::event_MouseMove(const sf::Event &event) {
	this->mouse_position.x = event.mouseMove.x / Graphics::READ->scaling_factor();
	this->mouse_position.y = event.mouseMove.y / Graphics::READ->scaling_factor();
}

void Input::event_KeyDown(const sf::Event &event) {
	this->keys_pressed[event.key.code] = true;
	this->keys_held[event.key.code] = true;
}
void Input::event_KeyUp(const sf::Event &event) {
	this->keys_released[event.key.code] = true;
	this->keys_held[event.key.code] = false;
}

void Input::event_ButtonDown(const sf::Event &event) {
	this->buttons_pressed[event.mouseButton.button] = true;
	this->buttons_held[event.mouseButton.button] = true;
}
void Input::event_ButtonUp(const sf::Event &event) {
	this->buttons_released[event.mouseButton.button] = true;
	this->buttons_held[event.mouseButton.button] = false;
}

bool Input::key_pressed(sf::Keyboard::Key key) {
	return this->keys_pressed[key];
}
bool Input::key_released(sf::Keyboard::Key key) {
	return this->keys_released[key];
}
bool Input::key_held(sf::Keyboard::Key key) {
	return this->keys_held[key];
}

bool Input::mouse_pressed(sf::Mouse::Button button) {
	return this->buttons_pressed[button];
}
bool Input::mouse_released(sf::Mouse::Button button) {
	return this->buttons_released[button];
}
bool Input::mouse_held(sf::Mouse::Button button) {
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
