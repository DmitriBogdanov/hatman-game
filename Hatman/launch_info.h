#pragma once

#include <SDL.h> // 'Uint32' type



// # LaunchInfo #
// - Contaons all info determined on a startup
struct LaunchInfo {

	// Window info
	void setInfo_Window(int width, int height, Uint32 flag);
	
	int window_width;
	int window_height;
	Uint32 window_flag;
};