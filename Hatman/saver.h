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
	Saver(const std::string &filePath = "temp/save.json"); // takes filepath to savefile

	static const Saver* READ; // used for aka 'global' access
	static Saver* ACCESS;

	void makeNewSave();
	void save();

	void record_Level(const std::string &name, const std::string &version = "default");
	void record_Player(); /// ADD OTHER STATS LATER

	std::string get_CurrentLevel() const;
	Vector2d get_PlayerPosition() const;

private:
	nlohmann::json state;

	std::string save_filepath;
};
