#include "tile_unique.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)

#include "game.h" // access to game state
#include "saver.h" // access to saving system (used by 'SaveOrb')
#include "controls.h" // access to control keys



/* ### CONTROLLERS ### */

typedef std::function<std::unique_ptr<Tile>(const Tileset&, int, const Vector2&)> make_derived_ptr;

// std::make_unique() wrapper
template<class UniqueTile>
std::unique_ptr<UniqueTile> make_derived(const Tileset &tileset, int id, const Vector2 &position) {
	return std::make_unique<UniqueTile>(tileset, id, position);
}

// !!! NAMES !!!
const std::unordered_map<std::string, make_derived_ptr> TILE_MAKERS = {
	{"", make_derived<Tile>},
	{"save_orb", make_derived<tiles::SaveOrb>}
	/// new tiles go there
};

std::unique_ptr<Tile> tiles::make_tile(const Tileset &tileset, int id, const Vector2 &position) {
	const std::string interactive_type = tileset.tileHas_Interaction(id)
		? tileset.tileGet_Interaction(id).interactive_type
		: "";

	return TILE_MAKERS.at(interactive_type)(tileset, id, position);
}



/* ### TILES ### */

// # SaveOrb #
tiles::SaveOrb::SaveOrb(const Tile &other) : Tile(other) {}
tiles::SaveOrb::SaveOrb(Tile &&other) : Tile(other) {}

tiles::SaveOrb::SaveOrb(const Tileset &tileset, int id, const Vector2 &position) : Tile(tileset, id, position) {}

bool tiles::SaveOrb::checkActivation() const {
	// Check if player is in range
	return this->interaction->actionbox.containsPoint(Game::ACCESS->level->player->position);
}
bool tiles::SaveOrb::checkTrigger() const {
	// Check if player pressed USE button
	return Game::ACCESS->input.key_pressed(Controls::READ->USE);
}

void tiles::SaveOrb::activate() {
	const auto text_position = this->position + Vector2d(16., -7.);
	this->popup_handle = Graphics::ACCESS->gui->make_line_centered("Press E", text_position);
	this->popup_handle.get().set_properties(colors::BLACK, false, false, 10);
}
void tiles::SaveOrb::deactivate() {
	this->popup_handle.erase();
}
void tiles::SaveOrb::trigger() {
	Saver::ACCESS->record_state();
	Saver::ACCESS->write(); 
}