#include "saver.h"

#include <fstream> // parsing from JSON (opening a file)
#include <iomanip> // used to "beautify" savefile JSON

#include "game.h" // access to game state



// # Saver #
namespace Saver_consts {
	const std::string FIRST_LEVEL = "shadow_test";
	constexpr auto FIRST_SPAWNPOINT = Vector2d(450., 240.);
}


const Saver* Saver::READ;
Saver* Saver::ACCESS;

Saver::Saver(const std::string &filePath) :
	save_filepath(filePath),
	save_is_present(false)
{
	this->READ = this;
	this->ACCESS = this;

	std::ifstream inFile(this->save_filepath);
	this->save_is_present = inFile.good();

	if (this->save_is_present)
		this->state = nlohmann::json::parse(inFile); // savefile is present => load
}

bool Saver::save_present() const {
	return this->save_is_present;
}


void Saver::create_new() {
	using namespace Saver_consts;

	this->state["player"]["current_level"] = FIRST_LEVEL;
	this->state["player"]["x"] = FIRST_SPAWNPOINT.x;
	this->state["player"]["y"] = FIRST_SPAWNPOINT.y;

	this->write();
}

void Saver::write() {
	std::ofstream file(this->save_filepath);
	file << std::setw(4) << this->state; // setw() 'pretifies' JSON so it is no a single unreadable line
	file.close();
}

// Recorders
void Saver::record_state() {
	this->state["player"]["current_level"] = Game::ACCESS->level->getName();
	this->state["player"]["x"] = Game::ACCESS->level->player->position.x;
	this->state["player"]["y"] = Game::ACCESS->level->player->position.y;
}

// Getters
std::string Saver::get_CurrentLevel() const {
	return this->state["player"]["current_level"].get<std::string>();
}

Vector2d Saver::get_PlayerPosition() const {
	return Vector2d(this->state["player"]["x"].get<double>(), this->state["player"]["y"].get<double>());
}