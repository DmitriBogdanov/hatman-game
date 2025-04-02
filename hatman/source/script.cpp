#include "script.h"

#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "graphics.h" // acess to GUI for text creation
#include "globalconsts.hpp" // fade duration during teleportation
#include "saver.h" // used by 'Checkpoint'



// # LevelChange #
scripts::LevelChange::LevelChange(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos) :
	hitbox(hitbox),
	goes_to_level(goesToLevel),
	goes_to_pos(goesToPos)
{}

void scripts::LevelChange::update([[maybe_unused]] Milliseconds elapsedTime) {
	if (Game::READ->level->player->solid->getHitbox().overlapsWithRect(this->hitbox)) {
		Game::ACCESS->request_levelChange(this->goes_to_level, this->goes_to_pos);
	}
}



// # LevelSwitch #
scripts::LevelSwitch::LevelSwitch(const dRect &hitbox, const std::string &goesToLevel, const Vector2 &goesToPos) :
	hitbox(hitbox),
	goes_to_level(goesToLevel),
	goes_to_pos(goesToPos)
{}

void scripts::LevelSwitch::update([[maybe_unused]] Milliseconds elapsedTime) {
	if (Game::READ->level->player->solid->getHitbox().overlapsWithRect(this->hitbox)
		&& Game::ACCESS->input.key_pressed(Controls::READ->USE)) {

		Game::ACCESS->request_levelChange(this->goes_to_level, this->goes_to_pos);
	}
}



// # Portal #
scripts::Portal::Portal(const dRect &hitbox, const Vector2 &goesToPos) :
	hitbox(hitbox),
	goes_to_pos(goesToPos),
	activated(false)
{}

void scripts::Portal::update([[maybe_unused]] Milliseconds elapsedTime) {
	if (!this->activated
		&& Game::READ->level->player->solid->getHitbox().overlapsWithRect(this->hitbox)
		&& Game::ACCESS->input.key_pressed(Controls::READ->USE))
	{
		this->activated = true;

		Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK.transparent(), colors::SH_BLACK, defaults::TRANSITION_FADE_DURATION);
		this->fade_timer.start(defaults::TRANSITION_FADE_DURATION);
	}

	if (this->activated && this->fade_timer.finished()) {
		this->activated = false;

		Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);

		Game::ACCESS->level->player->position = this->goes_to_pos;
		Game::ACCESS->level->player->cameraTrap_center();
	}
}



// # Hint #
namespace Hint_consts {
	constexpr Milliseconds TEXT_DELAY = 20.;
}

scripts::Hint::Hint(const dRect &hitbox, const dRect &textField, const std::string &text) :
	hitbox(hitbox),
	is_active(false),
	text_field(textField),
	text(text)
{}

scripts::Hint::~Hint() {
	this->popup_handle.erase();
}

void scripts::Hint::update([[maybe_unused]] Milliseconds elapsedTime) {
	// Check if player is in area currently
	const bool current_activation = this->hitbox.containsPoint(Game::READ->level->player->position);

	// Check if there is a change compared to previous frame
	const bool activation_has_changed = (this->is_active != current_activation);

	// If there was no change of state => nothing has to be done
	if (activation_has_changed) this->is_active = current_activation;
	else return;

	// Handle state change
	using namespace Hint_consts;

	if (this->is_active) {
		this->popup_handle = Graphics::ACCESS->gui->make_text(this->text, this->text_field);
		this->popup_handle.get().set_properties(colors::SH_YELLOW, false, false, TEXT_DELAY);
	}
	else {
		this->popup_handle.erase();
	}
}



// # Checkpoint #
scripts::Checkpoint::Checkpoint(const dRect &hitbox, const std::string &emits_flag) :
	hitbox(hitbox),
	was_triggered(false),
	emits_flag(emits_flag)
{}

void scripts::Checkpoint::update([[maybe_unused]] Milliseconds elapsedTime) {
	if (Game::READ->level->player->solid->getHitbox().overlapsWithRect(this->hitbox) && !this->was_triggered) {

		// Set it's own flag
		Flags::ACCESS->add(this->emits_flag); 
		
		// Save the game
		Saver::ACCESS->record_state(); // crutial to set flags BEFORE saving!
		Saver::ACCESS->write();

		this->was_triggered = true;
	}
}