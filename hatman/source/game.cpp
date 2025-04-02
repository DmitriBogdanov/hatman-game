#include "game.h"

#include <chrono>
#include <iostream>

#include <SFML/Audio.hpp>
#include <SFML/Audio/Music.hpp>

#include "UTL/log.hpp"

#include "graphics.h" // access to rendering updating
#include "saver.h" // access to save loading
#include "emit.h" // acess to 'EmitStorage' (DEV method _drawInfo())
#include "globalconsts.hpp"
#include "color.hpp" // coloring F3 GUI
#include "controls.h" // controls for GUI
#include "debug_tools.hpp" // for calling 'begin_new_frame()' function



// # Game #
const Game* Game::READ;
Game* Game::ACCESS;

Game::Game(int music_volume_setting, int sound_volume_setting, bool fps_counter_setting) :
	music_volume_mod(music_volume_setting / 10.),
	sound_volume_mod(sound_volume_setting / 10.),
	show_fps_counter(fps_counter_setting),
    toggle_F3(false),
	paused(false),
	timescale(1.),
	_true_time_elapsed(0.),
	_requested_go_to_main_menu(false),
	_requested_toggle_esc_menu(false),
    _requested_toggle_inventory(false),
	_requested_toggle_F3(false),
	_requested_exit_to_desktop(ExitCode::NONE),
	_requested_level_load_from_save(false),
	_requested_level_change(false),
	level_change_is_reload(false)
{
	std::cout << "Creating game object...\n";

	this->READ = this;
	this->ACCESS = this;

	// Load main menu
	std::cout << "Entering main menu...\n";
	this->request_goToMainMenu();

	Graphics::ACCESS->gui->FPSCounter_on();
	this->play_music("a_nights_respite.wav");
	
	Graphics::ACCESS->window.setKeyRepeatEnabled(false);

	/// Game loop moved to outside
}

Game::~Game() {}

void Game::play_music(const std::string &name, double volumeMod) {
	if (this->music_current_track == name) return; // don't repeat music if it's already playing

	this->music_current_track = name;

	// SFML version
	if (!music.openFromFile("content/audio/mx/" + name))
		std::cout << "Error: Could no open music file...\n";

	constexpr double SFML_MAX_VOLUME = 100; // SFML uses volume range [0, 100]
	const double total_volume = SFML_MAX_VOLUME * audio::MUSIC_BASE_VOLUME * Game::READ->music_volume_mod * volumeMod;
	const float clamped_volume = std::clamp(static_cast<float>(total_volume), 0.f, 100.f);
	music.setVolume(clamped_volume);
	music.setLoop(true); // for some reason music doesn't loop by default

	music.play();

	// SDL version
	//const int total_volume = static_cast<int>(SDL_MIX_MAXVOLUME * audio::MUSIC_BASE_VOLUME * this->music_volume_mod * volumeMod);
	//playMusic(("content/audio/mx/" + name).c_str(), total_volume);
}

bool Game::is_running() const {
	return static_cast<bool>(this->level);
}

void  Game::request_levelLoadFromSave() {
	if (this->_requested_level_load_from_save) return; // do nothing, process has already been initiated

	this->_requested_level_load_from_save = true;

	Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK.transparent(), colors::SH_BLACK, defaults::TRANSITION_FADE_DURATION);
	this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);
}

void Game::request_levelChange(const std::string &newLevel, const Vector2d newPosition) {
	if (_requested_level_change) return; // do nothing, process has already been initiated

	this->_requested_level_change = true;

	Graphics::ACCESS->gui->AllPlayerGUI_off();

	this->level_change_target = newLevel;
	this->level_change_position = newPosition;

	Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK.transparent(), colors::SH_BLACK, defaults::TRANSITION_FADE_DURATION);
	this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);
}

void Game::request_levelReload() {
	if (_requested_level_change) return; // do nothing, process has already been initiated

	this->_requested_level_change = true;
	this->level_change_is_reload = true;

	Graphics::ACCESS->gui->AllPlayerGUI_off();
		// note that player GUI crashes if it's active while player is dead => we want it turned off

	Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK.transparent(), colors::SH_BLACK, defaults::TRANSITION_FADE_DURATION);
	this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);
}

void Game::request_goToMainMenu() {
	if (this->_requested_go_to_main_menu) return; // do nothing, process has already been initiated

	this->_requested_go_to_main_menu = true;

	// Transition from game to menu needs smooth fade
	if (this->is_running()) {
		Graphics::ACCESS->gui->Fade_on(colors::ESC_MENU_FADE_COLOR, colors::SH_BLACK, defaults::TRANSITION_FADE_DURATION);
		this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);
	}
}

void Game::request_goToEndingScreen() {
	if (this->_requested_ending_screen) return; // do nothing, process has already been initiated

	this->_requested_ending_screen = true;

	// Transition needs smooth fade
	if (this->is_running()) {
		Graphics::ACCESS->gui->Fade_on(colors::ESC_MENU_FADE_COLOR, colors::SH_BLACK, defaults::GAME_ENDING_FADE_DURATION);
		this->smooth_transition_timer.start(defaults::GAME_ENDING_FADE_DURATION);
	}
}

void Game::request_toggleEscMenu() {
	this->_requested_toggle_esc_menu = true;
}

void Game::request_toggleInventory() {
	this->_requested_toggle_inventory = true;
}

void Game::request_toggleF3() {
	this->_requested_toggle_F3 = true;
}

void Game::request_exitToDesktop() {
	this->_requested_exit_to_desktop = ExitCode::EXIT;
}

void Game::request_exitToRestart() {
	this->_requested_exit_to_desktop = ExitCode::RESTART;
}

ExitCode Game::game_loop() {
	using clock = std::chrono::high_resolution_clock;
	auto frame_start = clock::now();

	auto &window = Graphics::ACCESS->window;

	while (window.isOpen()) {
		// Poll events to the input object
		sf::Event event;

		this->input.begin_new_frame();

		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::MouseMoved:
				this->input.event_MouseMove(event);
				break;
			case sf::Event::KeyPressed:
                UTL_LOG_INFO("Key pressed event.");
				this->input.event_KeyDown(event);
				break;
			case sf::Event::KeyReleased:
			    UTL_LOG_INFO("Key released event.");
				this->input.event_KeyUp(event);
				break;
			case sf::Event::MouseButtonPressed:
				this->input.event_ButtonDown(event);
				break;
			case sf::Event::MouseButtonReleased:
				this->input.event_ButtonUp(event);
				break;
			case sf::Event::Closed:
				return ExitCode::EXIT;
			default:
				break;
			}
		}

		// Measure frame time (in ms) and update 
		const auto frame_end = clock::now();
		const auto elapsed_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end - frame_start);
		Milliseconds elapsedTime = elapsed_time_ns.count() / 1e6; // ns to ms
		frame_start = frame_end; // next frame starts from the last end timestamp

		if (elapsedTime > performance::MAX_FRAME_TIME_MS) elapsedTime = performance::MAX_FRAME_TIME_MS;
			// fix for physics bugging out in low FPS moments
			// this means below 1000/40=25 FPS physics start to slow down 

		this->_true_time_elapsed = elapsedTime;
		
		const auto exit_code = this->handle_requests();
		if (exit_code != ExitCode::NONE) return exit_code;

		DEBUG_SINGLETON::get().begin_new_frame(); // reset internal counters

		this->update_everything(elapsedTime * this->timescale); // this is all there is to timescale mechanic
		this->draw_everything();
	}

	// Should be unreachable
	return ExitCode::EXIT;
}

ExitCode Game::handle_requests() {
	// Handle level change
	if (this->_requested_level_change && this->smooth_transition_timer.finished()) {
		if (this->level_change_is_reload) {
			Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);
			this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);

			this->_level_loadFromSave();

			Graphics::ACCESS->gui->LevelName_off(); // player position need to be updated for this text
				// avoids level name persisting in case player resets the game right after changing levels
		}
		else {
			Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);
			this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);

			this->_level_swapToTarget();

			Graphics::ACCESS->gui->LevelName_off(); // player position need to be updated for this text
			Graphics::ACCESS->gui->LevelName_on();
		}

		this->_requested_level_change = false;
	}

	// Handle loading from save
	if (this->_requested_level_load_from_save && this->smooth_transition_timer.finished()) {
		// Disable ending screen if it's the starting point
		Graphics::ACCESS->gui->EndingScreen_off();

		Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);
		this->smooth_transition_timer.start(defaults::TRANSITION_FADE_DURATION);

		this->_level_loadFromSave();

		Graphics::ACCESS->gui->MainMenu_off();

		this->_requested_level_load_from_save = false;
	}
		// NOTE: level change/load requests are handled first to ensure 'smooth_transition_timer'
		// restarts before any other 'smooth_transition_timer.finished()' checks

	// Handle esc menu toggle
	if (this->_requested_ending_screen)
		this->_requested_toggle_esc_menu = false; // protects from trying to turn on 'Esc' during some GUI transitions

	if (this->_requested_toggle_esc_menu && this->smooth_transition_timer.finished()) {
        // TEMP:
        UTL_LOG_WARN("Handling esc menu toggle");
		Graphics::ACCESS->gui->EscMenu_toggle();
		this->paused = !this->paused;

		this->_requested_toggle_esc_menu = false;
	}

	// Handle inventory toggle
	if (this->_requested_toggle_inventory && this->smooth_transition_timer.finished()) {
        // TEMP:
        UTL_LOG_WARN("Handling inventory toggle");
		Graphics::ACCESS->gui->Inventory_toggle();
		this->paused = !this->paused;

		this->_requested_toggle_inventory = false;
	}

	// Handle main menu toggle
	if (this->_requested_go_to_main_menu && this->smooth_transition_timer.finished()) {
		// Disable ending screen if it's the starting point
		Graphics::ACCESS->gui->EndingScreen_off();

		// Return to main menu from game
		if (this->is_running()) {
			this->level.reset();
			Graphics::ACCESS->gui->MainMenu_on();
		}
		// Start up main menu
		else {
			Graphics::ACCESS->gui->MainMenu_on();
		}

		Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);

		this->_requested_go_to_main_menu = false;
	}

	// Handle ending screen toggle
	if (this->_requested_ending_screen && this->smooth_transition_timer.finished()) {
		// Go to ending screen from game
		if (this->is_running()) {
			this->level.reset();
			Graphics::ACCESS->gui->EndingScreen_on();
		}

		Graphics::ACCESS->gui->Fade_on(colors::SH_BLACK, colors::SH_BLACK.transparent(), defaults::TRANSITION_FADE_DURATION);

		this->_requested_ending_screen = false;
	}

	// Handle F3 toggle
	if (this->_requested_toggle_F3) {
		this->toggle_F3 = !this->toggle_F3;

		this->_requested_toggle_F3 = false;
	}

	// Handle exit to desktop
	return this->_requested_exit_to_desktop;
}

void Game::update_everything(Milliseconds elapsedTime) {
	// Update during gameplay
	if (this->is_running() && !this->paused) {
		this->level->update(elapsedTime);

		Graphics::ACCESS->camera->position = this->level->player->cameraTrap_getPosition();
	}

	// Updated regardless
	Graphics::ACCESS->gui->update(elapsedTime);

	EmitStorage::ACCESS->update(elapsedTime);

	TimerController::ACCESS->update(elapsedTime);
}

void Game::draw_everything() {
	// 1) Clear window
	Graphics::ACCESS->window_clear();

	// 2) Draw everything
	if (this->is_running()) {
        this->level->draw();
    }

	if (this->is_running() && this->toggle_F3) { /// perhaps move to GUI
		this->_drawHitboxes();
		this->_drawInfo();
	}

	Graphics::ACCESS->gui->draw();

	// 3) Display drawn objects
	Graphics::ACCESS->window_display();
}

// Level loading/changing
void Game::_level_swapToTarget() {
    UTL_LOG_INFO("Swapping to level {", this->level_change_target, "}");
    
	auto extractedPlayer = this->level->_extractPlayer(); // extract player

	auto playerPtr = static_cast<ntt::player::Player*>(extractedPlayer.get());

	playerPtr->position = this->level_change_position;
		// set player position in a new level,
		// this should be done before spawning in a new level

	playerPtr->cameraTrap_center();
	Graphics::ACCESS->camera->position = playerPtr->cameraTrap_getPosition();
		// center camera at the player to prevent situation where player doesn't get
		// updated due to being too far away from camera

	this->level = std::make_unique<Level>(
		this->level_change_target,
		std::move(extractedPlayer)
	); // construct new level and transfer player to it

	this->_requested_level_change = false;

	Graphics::ACCESS->gui->AllPlayerGUI_on();
}

void Game::_level_loadFromSave() {
	const auto savedLevel = Saver::READ->get_CurrentLevel();
	const auto savedPosition = Saver::READ->get_PlayerPosition();
	auto savedInventory = Saver::READ->get_PlayerInventory(); // not const so we can std::move it
	auto savedFlags = Saver::READ->get_Flags(); // not const so we can std::move it

    UTL_LOG_INFO("Loading level {", savedLevel, "} from save");

	// Construct Player
	auto constructedPlayer = std::make_unique<ntt::player::Player>(savedPosition);
		
	constructedPlayer->cameraTrap_center();
	Graphics::ACCESS->camera->position = constructedPlayer->cameraTrap_getPosition();
		// center camera at the player to prevent situation where player doesn't get
		// updated due to being too far away from camera

	// Fill player inventory
	constructedPlayer->inventory = std::move(savedInventory);

	// Set flags
	Flags::ACCESS->flags = std::move(savedFlags);
    
	// Set level
	this->level = std::make_unique<Level>(
		savedLevel,
		std::move(constructedPlayer)
	);

	this->_requested_level_change = false;
	this->level_change_is_reload = false;

	Graphics::ACCESS->gui->AllPlayerGUI_on();
}

// Testing
void Game::_drawHitboxes() {
	const int BORDER_TEXTURE_SIZE = 16;

	// Draw tile hitboxes and actionboxes
	sf::Sprite tileHitboxBorder;
	tileHitboxBorder.setTexture(Graphics::ACCESS->getTexture("content/textures/hitbox_border_tile.png"));

	sf::Sprite tileActionboxBorder;
	tileActionboxBorder.setTexture(Graphics::ACCESS->getTexture("content/textures/actionbox_border_tile.png"));

	const auto& cameraPos = this->level->player->cameraTrap_getPosition();

	const Vector2 centerIndex = helpers::divide32(cameraPos);

	const int leftBound = std::max(centerIndex.x - performance::TILE_FREEZE_RANGE_X, 0);
	const int rightBound = std::min(centerIndex.x + performance::TILE_FREEZE_RANGE_X, this->level->getSizeX() - 1);
	const int upperBound = std::max(centerIndex.y - performance::TILE_FREEZE_RANGE_Y, 0);
	const int lowerBound = std::min(centerIndex.y + performance::TILE_FREEZE_RANGE_Y, this->level->getSizeY() - 1);

	// Update tiles
	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tile = this->level->getTile(X, Y);

			if (tile) {
				// Draw hibox if present
				if (tile->hitbox)
					for (const auto& hitboxRect : tile->hitbox->rectangles) {
						const dstRect destRect = hitboxRect.rect.to_dstRect();

						tileHitboxBorder.setPosition(
							static_cast<float>(destRect.x),
							static_cast<float>(destRect.y)
						);
						tileHitboxBorder.setScale(
							static_cast<float>(destRect.w / BORDER_TEXTURE_SIZE),
							static_cast<float>(destRect.h / BORDER_TEXTURE_SIZE)
						);

						Graphics::ACCESS->camera->draw_sprite(tileHitboxBorder);
					}
				// Draw actionbox if present
				if (tile->interaction) {
					const dstRect destRect = tile->interaction->actionbox.to_dstRect();

					tileActionboxBorder.setPosition(
						static_cast<float>(destRect.x),
						static_cast<float>(destRect.y)
					);
					tileActionboxBorder.setScale(
						static_cast<float>(destRect.w / BORDER_TEXTURE_SIZE),
						static_cast<float>(destRect.h / BORDER_TEXTURE_SIZE)
					);

					Graphics::ACCESS->camera->draw_sprite(tileActionboxBorder);
				}
			}
		}

	// Draw entity hitboxes
	sf::Sprite entityHitboxBorder;
	entityHitboxBorder.setTexture(Graphics::ACCESS->getTexture("content/textures/hitbox_border_entity.png"));

	for (const auto &entity : this->level->entities_solid) {
		const dstRect destRect = entity->solid->getHitbox().to_dstRect();

		entityHitboxBorder.setPosition(
			static_cast<float>(destRect.x),
			static_cast<float>(destRect.y)
		);
		entityHitboxBorder.setScale(
			static_cast<float>(destRect.w / BORDER_TEXTURE_SIZE),
			static_cast<float>(destRect.h / BORDER_TEXTURE_SIZE)
		);

		Graphics::ACCESS->camera->draw_sprite(entityHitboxBorder);
	}
}

void Game::_drawInfo() const {
	// Bypasses GUI text system and draws text directly through font

	Font* const font = Graphics::ACCESS->gui->fonts.at("BLOCKY").get();
	constexpr auto gap = Vector2d(2., 2.);
	constexpr double gapX = 80.;
	constexpr double gapY = 8.;

	// TEMP
	const auto &player = this->level->player;
	font->color_set(colors::SH_GREEN);
	// player state
	font->draw_line(gap + Vector2d(0 * gapX, 0 * gapY), "state:");
	font->draw_line(gap + Vector2d(1 * gapX, 0 * gapY), player->get_state_name());
	// position
	font->draw_line(gap + Vector2d(0 * gapX, 1 * gapY), "position:");
	font->draw_line(gap + Vector2d(1 * gapX, 1 * gapY), std::to_string(player->position.x));
	font->draw_line(gap + Vector2d(2 * gapX, 1 * gapY), std::to_string(player->position.y));
	// speed
	font->draw_line(gap + Vector2d(0 * gapX, 2 * gapY), "speed:");
	font->draw_line(gap + Vector2d(1 * gapX, 2 * gapY), std::to_string(player->solid->speed.x));
	font->draw_line(gap + Vector2d(2 * gapX, 2 * gapY), std::to_string(player->solid->speed.y));
	// acceleration
	font->draw_line(gap + Vector2d(0 * gapX, 3 * gapY), "acceleration:");
	font->draw_line(gap + Vector2d(1 * gapX, 3 * gapY), std::to_string(player->solid->acceleration.x));
	font->draw_line(gap + Vector2d(2 * gapX, 3 * gapY), std::to_string(player->solid->acceleration.y));
	// camera
	font->draw_line(gap + Vector2d(0 * gapX, 4 * gapY), "camera:");
	font->draw_line(gap + Vector2d(1 * gapX, 4 * gapY), std::to_string(Graphics::READ->camera->position.x));
	font->draw_line(gap + Vector2d(2 * gapX, 4 * gapY), std::to_string(Graphics::READ->camera->position.y));

	font->color_set(RGBColor(0, 0, 0));

	//const Vector2d start = Vector2d(2., 2.);
	//Vector2d cursor = start;

	//for (const auto &emit : EmitStorage::ACCESS->emits) {
	//	//const std::string &line = emit.first;

	//	font->draw_line(cursor, emit.first);
	//	//for (const auto &letter : line) { cursor = font->draw_symbol(cursor, letter); }
	//	cursor.x = start.x;
	//	cursor.y += font->get_font_monospace().y;
	//}
}


/// TESTING SETTINGS BUTTON
void Game::_reset_graphics() {
	//// Delete old Graphics
	//Graphics::ACCESS->~Graphics();

	//// Replace with a new objects
	//const int window_width = 640;
	//const int window_height = 360;

	//const auto screen_mode = sf::Style::Default;

	//Graphics graphics(window_width, window_height, screen_mode);

	//*(Graphics::ACCESS) = Graphics(window_width, window_height, screen_mode);
}