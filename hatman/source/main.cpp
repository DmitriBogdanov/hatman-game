// _______________________ INCLUDES _______________________

// Includes: std
#include <ctime>    // used to generate seed for random
#include <iostream> // logging to console

// Includes: dependencies
#include "firstparty/UTL/log.hpp"    // logging
#include "firstparty/UTL/predef.hpp" // debug info

// Includes: project
#include "graphics/graphics.h"    // Has a storage (initialized before start)
#include "objects/tile_base.h"    // Has a storage (initialized before start)
#include "systems/audio.h"        // Has a storage (initialized before start)
#include "systems/controls.h"     // Has a storage (initialized before start)
#include "systems/emit.h"         // Has a storage (initialized before start)
#include "systems/game.h"         // 'Game' class
#include "systems/saver.h"        // Has a storage (initialized before start)
#include "systems/timer.h"        // Has a storage (initialized before start)
#include "utility/launch_info.h"  // 'LaunchInfo' class

// ____________________ IMPLEMENTATION ____________________

int main() try {
    std::srand(static_cast<int>(std::time(nullptr))); // TODO: Remove this nonsense

    utl::log::add_file_sink("latest.log");
    utl::log::add_ostream_sink(std::cout);

	UTL_LOG_INFO("Binary info:\n", utl::predef::compilation_summary());

    ExitCode exit_code = ExitCode::NONE;

    while (exit_code != ExitCode::EXIT) {
        // Parse launch params from 'CONFIG.json'
        int         resolution_x;
        int         resolution_y;
        std::string screen_mode;
        int         music;
        int         sound;
        bool        fps_counter;
        std::string save_filepath;

        const bool config_found =
            config_parse(resolution_x, resolution_y, screen_mode, music, sound, fps_counter, save_filepath);

        // If no config exists, create the default one
        if (!config_found) {
            config_create_default();
            if (!config_parse(resolution_x, resolution_y, screen_mode, music, sound, fps_counter, save_filepath)) {
                UTL_LOG_ERR("Could not read default config.");
                return -1;
            }
        }

        // Initialize all the storage objects
        TimerController timerController; // [!] timers must be created first
        Graphics        graphics(resolution_x, resolution_y, convert_string_to_window_flags(screen_mode));
        Audio           audio(music, sound);
        TilesetStorage  tilesets;
        EmitStorage     emits;
        Flags           flags;
        Saver           saver(save_filepath);
        Controls        controls;
        Game            game(fps_counter);
        // from now on all these objects can be accessed through 'ClassName::ACCESS' / 'ClassName::READ'
        // anywhere that has their header included

        // Start the main loop
        exit_code = game.game_loop();

        UTL_LOG_NOTE("Exit code: ", static_cast<int>(exit_code));
    }
} catch (std::runtime_error& e) {
    UTL_LOG_ERR("ERROR: unhandled exception in main()");
    UTL_LOG_ERR("CAUGHT EXCEPTION: ", e.what());
}
