#include "systems/saver.h"

#include <fstream> // parsing from JSON (opening a file)
#include <iostream> // used to output messages when parsing
#include <iomanip> // used to "beautify" savefile JSON
#include <cstdio> // renaming files
#include <filesystem>

#include "systems/game.h" // access to game state
#include "objects/item_unique.h" // creation of items from name



// # Saver #
namespace Saver_consts {
	const std::string FIRST_LEVEL = "desolation";
	constexpr auto FIRST_SPAWNPOINT = Vector2d(160., 1296.);
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

	this->state["player"]["inventory"] = nlohmann::json::object();

	this->state["flags"] = nlohmann::json::array();

	this->write();
}

void Saver::write() {
	std::ofstream file(this->save_filepath);
	file << std::setw(4) << this->state; // setw() 'pretifies' JSON so it is no a single unreadable line
	file.close();
}

// Recorders
void Saver::record_state() {
	// Record player level and position
	this->state_set_level_and_position(Game::READ->level->getName(), Game::READ->level->player->position);

	// Record player inventory
	this->state_set_inventory(Game::READ->level->player->inventory);

	// Record flags
	this->state_set_flags(Flags::READ->flags);
}

void Saver::state_set_level_and_position(const std::string &level, const Vector2d &player_pos) {
	this->state["player"]["current_level"] = level;
	this->state["player"]["x"] = player_pos.x;
	this->state["player"]["y"] = player_pos.y;
}

void Saver::state_set_inventory(const Inventory &inventory) {
	auto items_array = nlohmann::json::array();

	for (const auto &stack : inventory.stacks) {
		auto stack_object = nlohmann::json::object();

		stack_object["name"] = stack.item().getName();
		stack_object["quantity"] = stack.quantity();

		items_array.push_back(stack_object);
	}

	this->state["player"]["inventory"] = std::move(items_array);
}

void Saver::state_set_flags(const std::unordered_set<Flag> &flags) {
	auto flags_array = nlohmann::json::array();

	for (const auto &flag : flags) flags_array.push_back(flag);

	this->state["flags"] = flags_array;
}

void Saver::backup_and_delete_current() {
	// Ensure directory exits
	std::filesystem::create_directory("backups");

	// Move current save
	const std::string BACKUP_FILEPATH = "backups/save.json";
	rename(this->save_filepath.data(), BACKUP_FILEPATH.data());

	this->state.clear();
	this->save_is_present = false;
}

// Getters
std::string Saver::get_CurrentLevel() const {
	return this->state["player"]["current_level"].get<std::string>();
}

Vector2d Saver::get_PlayerPosition() const {
	return Vector2d(this->state["player"]["x"].get<double>(), this->state["player"]["y"].get<double>());
}

Inventory Saver::get_PlayerInventory() const {
	Inventory parsed_inventory;

	const nlohmann::json &items_array = this->state["player"]["inventory"];

	for (const auto &stack_object : items_array) {
		const auto item = items::make_item(stack_object["name"].get<std::string>());

		parsed_inventory.addItem(*item, stack_object["quantity"].get<int>());
	}

	return parsed_inventory;
}

std::unordered_set<Flag> Saver::get_Flags() const {
	std::unordered_set<Flag> flags;

	for (const auto &flag_node : this->state["flags"]) flags.insert(flag_node.get<std::string>());

	return flags;
}



// # Config #
void config_create(int resolution_x, int resolution_y, const std::string &screen_mode, int music, int sound, bool fps_counter, const std::string &save_filepath) {
	nlohmann::json json;

	json["resolution_x"] = resolution_x;
	json["resolution_y"] = resolution_y;
	json["screen_mode"] = screen_mode;
	json["music"] = music;
	json["sound"] = sound;
	json["fps_counter"] = fps_counter;
	json["save_filepath"] = save_filepath;

	json["_COMMENTS_"] = "Non-standard resolutions can be selected manually through config. Options for 'screen_mode': 1) WINDOW; 2) BORDERLESS; 3) FULLSCREEN.";

	// Create/rewrite config file
	std::ofstream file(CONFIG_PATH);
	file << std::setw(4) << json; // setw() 'prettifies' JSON so it is not a single unreadable line
	file.close();
}

void config_create_default() {

	std::cout << "Creating default config...\n";

	config_create(
		1280,
		720,
		"WINDOW",
		10,
		10,
		false,
		"temp/save.json"
	);
}

bool config_parse(int &resolution_x, int &resolution_y, std::string &screen_mode, int &music, int &sound, bool &fps_counter, std::string &save_filepath) {
	// Load 'CONFIG.json'
	std::cout << "Parsing config...\n";

	std::ifstream configFile(CONFIG_PATH);
	if (!configFile.good()) {
		std::cout << "Note: Could no find CONFIG.json\n";
		return false;
	};

	nlohmann::json config_json = nlohmann::json::parse(configFile); // savefile is present => load

	// Parse launch params from 'CONFIG.json'
	resolution_x = config_json["resolution_x"];
	resolution_y = config_json["resolution_y"];
	screen_mode = config_json["screen_mode"];
	music = config_json["music"];
	sound = config_json["sound"];
	fps_counter = config_json["fps_counter"];
	save_filepath = config_json["save_filepath"];

	// Return success
	return true;
}

sf::Uint32 convert_string_to_window_flags(const std::string &str) {
	if (str == "WINDOW") return sf::Style::Default;
	if (str == "BORDERLESS") return sf::Style::None;
	if (str == "FULLSCREEN") return sf::Style::Fullscreen;
	return sf::Style::Default;
}