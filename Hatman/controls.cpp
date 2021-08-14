#include "controls.h"



// # Controls #
const Controls* Controls::READ;
Controls* Controls::ACCESS;

Controls::Controls() {
	this->READ = this;
	this->ACCESS = this;  

	this->LEFT = SDL_SCANCODE_A;
	this->RIGHT = SDL_SCANCODE_D;
	this->UP = SDL_SCANCODE_W;
	this->DOWN = SDL_SCANCODE_S;

	this->CHAIN = SDL_BUTTON_LEFT;
	this->SHIFT = SDL_SCANCODE_LSHIFT;
	this->SKILL = SDL_SCANCODE_C;

	this->JUMP = SDL_SCANCODE_SPACE;
	this->USE = SDL_SCANCODE_E;

	this->INVENTORY = SDL_SCANCODE_I;
}