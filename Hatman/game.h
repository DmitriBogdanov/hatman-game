#pragma once

#include "timer.h" // 'Timer' class, 'Milliseconds' type
#include "input.h" // 'Input' class
#include "level.h" // 'Level' class




// # Game #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Holds the game loop
// - Handles most high-level logic
class Game {
public:
	Game(); // inits SDL

	~Game(); // quits SDL

	static const Game* READ; // used for aka 'global' access
	static Game* ACCESS;

	void play_music(const std::string &name, double volumeMod = 1.);
	void play_sound(const std::string &name, double volumeMod = 1.);

	bool is_running() const; // true when level is loaded and running, false in main menu and etc

	void request_levelLoadFromSave();
	void request_levelChange(const std::string &newLevel, const Vector2d newPosition); // changes level to given, version is loaded from save
	void request_levelReload();

	void request_goToMainMenu();
	void request_toggleEscMenu();
	void request_toggleF3();
	void request_exitToDesktop();

	bool paused;
	double timescale;

	std::unique_ptr<Level> level;

	Input input;

	Milliseconds _true_time_elapsed;
		// used by some GUI things that calculate time independent from timescale
		// mostly here for FPS counter

private:
	void game_loop();

	bool handle_requests(); // exit game if returns false
	void update_everything(Milliseconds elapsedTime); // updates everything
	void draw_everything(); // draws everything, not const because level can add nullptrs to the tilemap during drawing

	// GUI reauests
	bool _requested_go_to_main_menu;
	bool _requested_toggle_esc_menu;
	bool _requested_toggle_F3;
	bool _requested_exit_to_desktop;
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

	// Testing
	bool toggle_F3;
	void _drawHitboxes(); // shows an outline of all hitboxes and tule actionboxes
	///void _drawLevelobjects() const; // shows an outline of all objects on a level
	void _drawInfo() const; // shows content of EmitStorage
};