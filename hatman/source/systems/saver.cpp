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
    
    this->save_is_present = std::filesystem::exists(this->save_filepath);

	if (this->save_is_present) // savefile is present => load
		this->state = utl::json::from_file(this->save_filepath); 
}

bool Saver::save_present() const {
	return this->save_is_present;
}


void Saver::create_new() {
	using namespace Saver_consts;

	this->state["player"]["current_level"] = FIRST_LEVEL;

	this->state["player"]["x"] = FIRST_SPAWNPOINT.x;
	this->state["player"]["y"] = FIRST_SPAWNPOINT.y;

	this->state["player"]["inventory"] = utl::json::Array{};

	this->state["flags"] = utl::json::Array{};

	this->write();
}

void Saver::write() {
    this->state.to_file(this->save_filepath);
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
	auto items_array = utl::json::Array{};

	for (const auto &stack : inventory.stacks) {
		auto stack_object = utl::json::Object{};

		stack_object["name"] = stack.item().getName();
		stack_object["quantity"] = stack.quantity();

		items_array.push_back(stack_object);
	}

	this->state["player"]["inventory"] = std::move(items_array);
}

void Saver::state_set_flags(const std::unordered_set<Flag> &flags) {
	auto flags_array = utl::json::Array{};

	for (const auto &flag : flags) flags_array.push_back(flag);

	this->state["flags"] = flags_array;
}

void Saver::backup_and_delete_current() {
	// Ensure directory exits
	std::filesystem::create_directory("backups");

	// Move current save
	const std::string BACKUP_FILEPATH = "backups/save.json";
	rename(this->save_filepath.data(), BACKUP_FILEPATH.data());
    
    this->state = utl::json::Node{};
	this->save_is_present = false;
}

// Getters
std::string Saver::get_CurrentLevel() const {
	return this->state["player"]["current_level"].get_string();
}

Vector2d Saver::get_PlayerPosition() const {
	return Vector2d(this->state["player"]["x"].get_number(), this->state["player"]["y"].get_number());
}

Inventory Saver::get_PlayerInventory() const {
	Inventory parsed_inventory;

	const auto &items_array = this->state["player"]["inventory"].get_array();

	for (const auto &stack_object : items_array) {
		const auto item = items::make_item(stack_object["name"].get_string());

		parsed_inventory.addItem(*item, stack_object["quantity"].get_number());
	}

	return parsed_inventory;
}

std::unordered_set<Flag> Saver::get_Flags() const {
	std::unordered_set<Flag> flags;

	for (const auto &flag_node : this->state["flags"].get_array()) flags.insert(flag_node.get_string());

	return flags;
}



// # Config #
void config_create(int resolution_x, int resolution_y, const std::string &screen_mode, int music, int sound, bool fps_counter, const std::string &save_filepath) {
	utl::json::Node json;

	json["resolution_x"] = resolution_x;
	json["resolution_y"] = resolution_y;
	json["screen_mode"] = screen_mode;
	json["music"] = music;
	json["sound"] = sound;
	json["fps_counter"] = fps_counter;
	json["save_filepath"] = save_filepath;

	json["_COMMENTS_"] = "Non-standard resolutions can be selected manually through config. Options for 'screen_mode': 1) WINDOW; 2) BORDERLESS; 3) FULLSCREEN.";

	// Create/rewrite config file
    json.to_file(CONFIG_PATH);
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
    
    utl::json::Node config_json;
    
    try {
	    config_json = utl::json::from_file(CONFIG_PATH); // savefile is present => load
    } catch (std::runtime_error& e) {
        std::cout << "CAUGHT EXCEPTION: " << e.what() << "\n";
        std::cout << "NOTE: Could not find CONFIG.json\n";
        return false;
    }

	// Parse launch params from 'CONFIG.json'
	resolution_x = config_json["resolution_x"].get_number();
	resolution_y = config_json["resolution_y"].get_number();
	screen_mode = config_json["screen_mode"].get_string();
	music = config_json["music"].get_number();
	sound = config_json["sound"].get_number();
	fps_counter = config_json["fps_counter"].get_bool();
	save_filepath = config_json["save_filepath"].get_string();

	// Return success
	return true;
}

sf::Uint32 convert_string_to_window_flags(const std::string &str) {
	if (str == "WINDOW") return sf::Style::Default;
	if (str == "BORDERLESS") return sf::Style::None;
	if (str == "FULLSCREEN") return sf::Style::Fullscreen;
	return sf::Style::Default;
}