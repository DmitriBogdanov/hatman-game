#pragma once

#include <string> // related type
#include <unordered_set> // flags are stored as a set
#include "thirdparty/nlohmann.hpp" // parsing from JSON, 'nlohmann::json' type

#include "utility/geometry.h" // geometry types
#include "modules/inventory.h" // since we have to parse inventory from save
#include "systems/flags.h" // parsing of flags from the savefile



// # Saver #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Opens save files
// - Records and saves game progress
class Saver {
public:
	Saver(const std::string &filePath); // takes filepath to savefile

	static const Saver* READ; // used for aka 'global' access
	static Saver* ACCESS;

	bool save_present() const; // return whether save exists

	void create_new();

	void write(); // writes state to file

	void record_state();

	// Parts of 'record_state()', exposed to allow manual saving while game is not running
	void state_set_level_and_position(const std::string &level, const Vector2d &player_pos); // used to return fron ending screen
	void state_set_inventory(const Inventory &inventory);
	void state_set_flags(const std::unordered_set<Flag> &flags);

	void backup_and_delete_current(); // creates a copy of current save and cleans 'selected' file

	std::string get_CurrentLevel() const;
	Vector2d get_PlayerPosition() const;
	Inventory get_PlayerInventory() const;
	std::unordered_set<Flag> get_Flags() const;

private:
	std::string save_filepath;

	bool save_is_present;

	nlohmann::json state;
};


// # Config #
const std::string CONFIG_PATH = "CONFIG.json";

void config_create(
	int resolution_x,
	int resolution_y,
	const std::string &screen_mode,
	int music,
	int sound,
	bool fps_counter,
	const std::string &save_filepath
);

void config_create_default();

bool config_parse(
	int &resolution_x,
	int &resolution_y,
	std::string &screen_mode,
	int &music,
	int &sound,
	bool &fps_counter,
	std::string &save_filepath
);
	// outputs true when successfull

sf::Uint32 convert_string_to_window_flags(const std::string &str);
