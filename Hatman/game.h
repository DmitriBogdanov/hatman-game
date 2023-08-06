#pragma once

#include <SFML/Audio.hpp>

#include "timer.h" // 'Timer' class, 'Milliseconds' type
#include "input.h" // 'Input' class
#include "level.h" // 'Level' class


enum class ExitCode {
	NONE = 0,
	EXIT = 1,
	RESTART = 2
};


// # Game #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Holds the game loop
// - Handles most high-level logic
class Game {
public:
	Game(int music_volume_setting, int sound_volume_setting, bool fps_counter_setting); // inits SDL

	~Game(); // quits SDL

	static const Game* READ; // used for aka 'global' access
	static Game* ACCESS;

	ExitCode game_loop(); // called from outside to start the game loop

	void play_music(const std::string &name, double volumeMod = 1.); // additional volume mod to adjust particular sounds
	void play_sound(const std::string &name, double volumeMod = 1.);

	double music_volume_mod;
	double sound_volume_mod;

	std::string music_current_track;

	sf::Music music;

	bool show_fps_counter;
	bool toggle_F3;

	bool is_running() const; // true when level is loaded and running, false in main menu and etc

	void request_levelLoadFromSave();
	void request_levelChange(const std::string &newLevel, const Vector2d newPosition); // changes level to given, version is loaded from save
	void request_levelReload();

	void request_goToMainMenu();
	void request_goToEndingScreen();
	void request_toggleEscMenu();
	void request_toggleInventory();
	void request_toggleF3();
	void request_exitToDesktop();
	void request_exitToRestart();

	bool paused;
	double timescale;

	std::unique_ptr<Level> level;

	Input input;

	Milliseconds _true_time_elapsed;
		// used by some GUI things that calculate time independent from timescale
		// mostly here for FPS counter

	void _reset_graphics();

private:

	ExitCode handle_requests(); // exit game if returns false
	void update_everything(Milliseconds elapsedTime); // updates everything
	void draw_everything(); // draws everything, not const because level can add nullptrs to the tilemap during drawing

	// GUI requests
	bool _requested_go_to_main_menu;
	bool _requested_ending_screen;
	bool _requested_toggle_esc_menu;
	bool _requested_toggle_inventory;
	bool _requested_toggle_F3;
	ExitCode _requested_exit_to_desktop;
	bool _requested_level_load_from_save;

	bool _requested_level_change;

	bool level_change_is_reload; // true if level change is reload from save
	std::string level_change_target; // name of the level to change
	Vector2d level_change_position;
		// player position on a new level
		// note that if level is loaded frome save these parameters are not used in any way

	Timer smooth_transition_timer; // waits for fade animations to finish

	void _level_swapToTarget();
	void _level_loadFromSave();

	// Testing (F3 toggle)
	void _drawHitboxes(); // shows an outline of all hitboxes and tule actionboxes
	///void _drawLevelobjects() const; // shows an outline of all objects on a level
	void _drawInfo() const; // shows content of EmitStorage
};