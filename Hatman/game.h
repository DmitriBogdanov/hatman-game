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

	void play_Music(const std::string &name);
	void play_Sound(const std::string &name);

	void request_LevelChange(const std::string &newLevel, const Vector2d newPosition); // changes level to given, version is loaded from save
	void request_LevelReload();

	double timescale = 1;

	Level level;

	Input input;

	Milliseconds _true_time_elapsed = 0;
		// used by some GUI things that calculate time independent from timescale
		// mostly here for FPS counter

private:
	void gameLoop();

	void updateGame(Milliseconds elapsedTime); // updates everything
	void drawGame(); // draws everything, not const because level can add nullptrs to the tilemap during drawing

	// Level loading/changing
	bool level_change_requested = false; // true if level change was requested
	bool level_change_is_reload = false; // true if level change is reload from save

	std::string level_change_target; // name of the level to change
	Vector2d level_change_position; // player position on a new level
		// note that if level is loaded frome save these parameters are not used in any way

	Timer level_change_timer; // waits for level change animation to finish

	void _level_swapToTarget();
	void _level_loadFromSave();

	// Testing
	bool toggle_F3;
	void _drawHitboxes(); // shows an outline of all hitboxes and tule actionboxes
	///void _drawLevelobjects() const; // shows an outline of all objects on a level
	void _drawInfo() const; // shows content of EmitStorage
};