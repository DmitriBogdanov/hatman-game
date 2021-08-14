#pragma once

#include <SDL_events.h> // 'SDL_Scancode' type



class Controls {
public:
	Controls();

	static const Controls* READ;
	static Controls* ACCESS;

	// GUI controls
	Uint8 LMB;
	SDL_Scancode ESC;
	SDL_Scancode F3;

	// Game controls
	SDL_Scancode LEFT;
	SDL_Scancode RIGHT;
	SDL_Scancode UP;
	SDL_Scancode DOWN;

	Uint8 CHAIN;
	SDL_Scancode SHIFT;
	SDL_Scancode SKILL;

	SDL_Scancode JUMP;
	SDL_Scancode USE;

	SDL_Scancode INVENTORY;
};