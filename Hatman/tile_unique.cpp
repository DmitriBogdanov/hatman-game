#include "tile_unique.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)

#include "game.h" // access to game state
#include "saver.h" // access to saving system (used by 'SaveOrb')
#include "controls.h" // access to control keys
#include "globalconsts.hpp" // tile size const



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
	{"save_orb", make_derived<tiles::SaveOrb>},
	{"portal", make_derived<tiles::Portal>}
	/// new tiles go there
};

std::unique_ptr<Tile> tiles::make_tile(const Tileset &tileset, int id, const Vector2 &position) {
	const std::string interactive_type = tileset.has_tile_interaction(id)
		? tileset.get_tile_interaction(id).interactive_type
		: "";

	return TILE_MAKERS.at(interactive_type)(tileset, id, position);
}



/* ### TILES ### */

// # SaveOrb #
namespace SaveOrb_consts {
	constexpr Milliseconds TEXT_DELAY = 20.;
	constexpr auto TEXT_ALIGNMENT = Vector2d(natural::TILE_SIZE / 2., -6.);

	const std::string ACTIVATED_TEXT = "Save progress?";
	const std::string TRIGGERED_TEXT = "Progress saved.";
}

tiles::SaveOrb::SaveOrb(const Tileset &tileset, int id, const Vector2 &position) :
	Tile(tileset, id, position),
	activation_sound("gui_click.wav")
{}

tiles::SaveOrb::~SaveOrb() {
	this->popup_handle.erase();
}

bool tiles::SaveOrb::checkActivation() const {
	// Check if player is in range
	return this->interaction->actionbox.containsPoint(Game::READ->level->player->position);
}
bool tiles::SaveOrb::checkTrigger() const {
	// Check if player pressed USE button
	return Game::ACCESS->input.key_pressed(Controls::READ->USE);
}

void tiles::SaveOrb::activate() {
	using namespace SaveOrb_consts;

	const auto text_position = this->position + TEXT_ALIGNMENT;

	this->popup_handle = Graphics::ACCESS->gui->make_line_centered(ACTIVATED_TEXT, text_position);
	this->popup_handle.get().set_properties(colors::SH_YELLOW, false, false, TEXT_DELAY);
}
void tiles::SaveOrb::deactivate() {
	this->popup_handle.erase();
}
void tiles::SaveOrb::trigger() {
	using namespace SaveOrb_consts;

	this->activation_sound.play();

	Saver::ACCESS->record_state();
	Saver::ACCESS->write();

	const auto text_position = this->position + TEXT_ALIGNMENT;

	this->popup_handle.erase();
	this->popup_handle = Graphics::ACCESS->gui->make_line_centered(TRIGGERED_TEXT, text_position);
	this->popup_handle.get().set_properties(colors::SH_YELLOW, false, false, TEXT_DELAY);
}



// # Portal #
namespace Portal_consts {
	constexpr Milliseconds TEXT_DELAY = 20.;
	constexpr auto TEXT_ALIGNMENT = Vector2d(0., -10.);

	const std::string ACTIVATED_TEXT = "Use portal?";
}

tiles::Portal::Portal(const Tile &other) : Tile(other) {}
tiles::Portal::Portal(Tile &&other) : Tile(other) {}

tiles::Portal::Portal(const Tileset &tileset, int id, const Vector2 &position) : Tile(tileset, id, position) {}

tiles::Portal::~Portal() {
	this->popup_handle.erase();
}

bool tiles::Portal::checkActivation() const {
	// Check if player is in range
	return this->interaction->actionbox.containsPoint(Game::READ->level->player->position);
}
bool tiles::Portal::checkTrigger() const {
	// Check if player pressed USE button
	return Game::ACCESS->input.key_pressed(Controls::READ->USE);
}

void tiles::Portal::activate() {
	using namespace Portal_consts;

	const auto text_position = this->interaction->actionbox.getTopMiddlepoint() + TEXT_ALIGNMENT;

	this->popup_handle = Graphics::ACCESS->gui->make_line_centered(ACTIVATED_TEXT, text_position);
	this->popup_handle.get().set_properties(colors::SH_YELLOW, false, false, TEXT_DELAY);
}
void tiles::Portal::deactivate() {
	this->popup_handle.erase();
}
void tiles::Portal::trigger() {
	/// LOOK FOR FX
	///Game::ACCESS->play_sound("gui_click.wav");
}