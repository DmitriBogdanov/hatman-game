// Entry point

///#define _CRTDBG_MAP_ALLOC /// MEMORY LEAK DETECTION
///#include <stdlib.h> /// CURRENT LEAK IS 16 BYTES
///#include <crtdbg.h>

#define SDL_MAIN_HANDLED

#ifdef _DEBUG
#define SHOW_DEBUG_CONSOLE
#endif

#include <iostream> // Text to console (TEMP)
#include <time.h> // used to generate seed for random
#include <fstream>

#include "graphics.h" // Has a storage (initialized before start)
#include "audio.h" // Has a storage (initialized before start)
#include "tile_base.h" // Has a storage (initialized before start)
#include "emit.h" // Has a storage (initialized before start)
#include "saver.h" // Has a storage (initialized before start)
#include "timer.h" // Has a storage (initialized before start)
#include "controls.h" // Has a storage (initialized before start)

#include "launch_info.h" // 'LaunchInfo' class (creation of such object)
#include "game.h" // 'Game' class
#include "hide_console.h"



int main(int argc, char* argv[]) {
	srand(static_cast<int>(time(nullptr)));

	#ifdef SHOW_DEBUG_CONSOLE
	show_console();
	#else
	hide_console();
	#endif

	std::cout << "- Execution log -" << std::endl;	
	
	ExitCode exit_code = ExitCode::NONE;

	while (exit_code != ExitCode::EXIT) {	
		// Parse launch params from 'CONFIG.json'
		int resolution_x;
		int resolution_y;
		std::string screen_mode;
		int music;
		int sound;
		bool fps_counter;
		std::string save_filepath;

		const bool config_found = config_parse(resolution_x, resolution_y, screen_mode, music, sound, fps_counter, save_filepath);

		// If no config exists, create the default one
		if (!config_found) {
			config_create_default();
			if (!config_parse(resolution_x, resolution_y, screen_mode, music, sound, fps_counter, save_filepath)) {
				std::cout << "Error: Could not read default config.";
				return -1;
			}
		}
		 
		// These objects are storages that can be accessed in any file with a corresponding header included
		Graphics graphics(resolution_x, resolution_y, convert_string_to_window_flags(screen_mode)); // From now on this object can be accessed through 'Graphics::ACCESS'
		Audio audio;
		TilesetStorage tilesets; // From now on this object can be accessed through 'TilesetStorage::ACCESS'
		EmitStorage emits; // From now on this object can be accessed through 'EmitStorage::ACCESS'
		Flags flags; // From now on this object can be accessed through 'Flags::ACCESS'
		Saver saver(save_filepath); // From now on this object can be accessed through 'Saver::ACCESS'
		TimerController timerController;
		Controls controls;

		Game game(music, sound, fps_counter);

		// Start the main loop
		exit_code = game.game_loop();

		std::cout << "Exit code: " << static_cast<int>(exit_code) << "\n";
	}

	///_CrtDumpMemoryLeaks(); /// MEMORY LEAK DETECTION
	return 0;
}
