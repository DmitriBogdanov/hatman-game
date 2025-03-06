#pragma once

#include "hide_console.h"



#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#endif


void show_console() {
	#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	#endif
}

void hide_console() {
	#ifdef _WIN32
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	#endif
}