#pragma once

#include <string> // related type
#include "nlohmann_external.hpp" // parsing from JSON, 'nlohmann::json' type

#include "geometry_utils.h" // geometry types



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

	std::string get_CurrentLevel() const;
	Vector2d get_PlayerPosition() const;

private:
	std::string save_filepath;

	bool save_is_present;

	nlohmann::json state;
};
