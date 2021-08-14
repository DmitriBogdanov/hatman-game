// Entry point

///#define _CRTDBG_MAP_ALLOC /// MEMORY LEAK DETECTION
///#include <stdlib.h> /// CURRENT LEAK IS 16 BYTES
///#include <crtdbg.h>

#define SDL_MAIN_HANDLED

#include <iostream> // Text to console (TEMP)
#include <time.h> // used to generate seed for random

#include "graphics.h" // Has a storage (initialized before start)
#include "tile_base.h" // Has a storage (initialized before start)
#include "emit.h" // Has a storage (initialized before start)
#include "saver.h" // Has a storage (initialized before start)
#include "timer.h" // Has a storage (initialized before start)
#include "controls.h" // Has a storage (initialized before start)

#include "launch_info.h" // 'LaunchInfo' class (creation of such object)
#include "game.h" // 'Game' class



int main(int argc, char* argv[]) {
	SDL_SetMainReady();

	srand(static_cast<int>(time(nullptr)));

	std::cout << "Start game in fullscreen mode? (Y/N)" << std::endl;
	std::string userInput;
	LaunchInfo launchInfo;

	// Variables related to debug commands
	std::string _savename = "save";

	while (true) {
		std::cin >> userInput;
		if (userInput == "Y" || userInput == "y" || userInput == "Yes" || userInput == "yes") {
			// No goddamn idea why, but borderless setting stretches the image
			//launchInfo.setInfo_Window(1920, 1080, 0);
			launchInfo.setInfo_Window(1920, 1080, SDL_WINDOW_FULLSCREEN_DESKTOP);
			//launchInfo.setInfo_Window(1920, 1080, SDL_WINDOW_BORDERLESS);
			break;
		}
		else if (userInput == "N" || userInput == "n" || userInput == "No" || userInput == "no") {
			launchInfo.setInfo_Window(2 * 640, 2 * 360, 0);
			break;
		}
		else if (userInput == "/debug") {
			std::cout << "#   DEBUG MODE   #" << std::endl;
			while (true) {
				std::cin >> userInput;
				if (userInput == "/end") { break; }
				else if (userInput == "/setsave") {
					std::cin >> _savename;
					std::cout << "$ Savefile selected" << std::endl;
				}
			}
		}
		else {
			std::cout << "Incorrect input, please try again" << std::endl;
		}
	}

	{
		// These objects are storages that can be accessed in any file with a corresponding header included
		Graphics graphics(launchInfo); // From now on this object can be accessed through 'Graphics::ACCESS'
		TilesetStorage tilesets; // From now on this object can be accessed through 'TilesetStorage::ACCESS'
		EmitStorage emits; // From now on this object can be accessed through 'EmitStorage::ACCESS'
		Saver saver("temp/" + _savename + ".json"); // From now on this object can be accessed through 'Saver::ACCESS'
		TimerController timerController;
		Controls controls;

		Game game;
	}

	///_CrtDumpMemoryLeaks(); /// MEMORY LEAK DETECTION
	return 0;
}
