#include "game.h"

#include <SDL.h> // 'SDL_Init()' and SDL event system
#include "simple_audio.h"

#include "graphics.h" // access to rendering updating
#include "saver.h" // access to save loading
#include "emit.h" // acess to 'EmitStorage' (DEV method _drawInfo())
#include "globalconsts.hpp"
#include "color.hpp" // coloring F3 GUI
#include "controls.h" // controls for GUI



// # Game #
const Game* Game::READ;
Game* Game::ACCESS;

Game::Game() :
	paused(false),
	timescale(1.),
	_true_time_elapsed(0.),
	_requested_toggle_esc_menu(false),
	_requested_toggle_F3(false),
	_requested_exit_to_desktop(false),
	_requested_level_change(false),
	level_change_is_reload(false),
	toggle_F3(false)
{
	this->READ = this;
	this->ACCESS = this;

	SDL_Init(SDL_INIT_EVERYTHING);
	initAudio();

	this->_level_loadFromSave();

	// Turn GUI that isn't player GUI
	Graphics::ACCESS->gui->FPSCounter_on();

	this->play_music("a_nights_respite.wav");

	Graphics::ACCESS->camera->zoom = 0.5;

	// Start the game loop
	this->game_loop();
}

Game::~Game() {
	endAudio();
	SDL_Quit();
}

void Game::play_music(const std::string &name, double volumeMod) {
	playMusic(("content/audio/mx/" + name).c_str(), static_cast<int>(SDL_MIX_MAXVOLUME * audio::MUSIC_VOLUME * volumeMod));
}

void Game::play_sound(const std::string &name, double volumeMod) {
	playSound(("content/audio/fx/" + name).c_str(), static_cast<int>(SDL_MIX_MAXVOLUME * audio::FX_VOLUME * volumeMod));
}

void Game::request_levelChange(const std::string &newLevel, const Vector2d newPosition) {
	if (_requested_level_change) return; // do nothing, process has already been initiated

	Graphics::ACCESS->gui->AllPlayerGUI_off();

	Graphics::ACCESS->gui->Fade_on(colors::BLACK.transparent(), colors::BLACK, defaults::LEVEL_CHANGE_FADE_DURATION);

	this->_requested_level_change = true;

	this->level_change_target = newLevel;
	this->level_change_position = newPosition;

	this->level_change_timer.start(defaults::LEVEL_CHANGE_FADE_DURATION);
}

void Game::request_levelReload() {
	if (_requested_level_change) return; // do nothing, process has already been initiated

	Graphics::ACCESS->gui->AllPlayerGUI_off();
		// note that player GUI crashes if it's active while player is dead => we want it turned off

	Graphics::ACCESS->gui->Fade_on(colors::BLACK.transparent(), colors::BLACK, defaults::LEVEL_CHANGE_FADE_DURATION);

	this->_requested_level_change = true;
	this->level_change_is_reload = true;

	this->level_change_timer.start(defaults::LEVEL_CHANGE_FADE_DURATION);
}

void Game::request_toggleEscMenu() {
	this->_requested_toggle_esc_menu = true;
}

void Game::request_toggleF3() {
	this->_requested_toggle_F3 = true;
}

void Game::request_exitToDesktop() {
	this->_requested_exit_to_desktop = true;
}

void Game::game_loop() {
	const auto FREQUENCY = static_cast<double>(SDL_GetPerformanceFrequency()); // platform specific frequency, does not change during runtime
	auto LAST_UPDATE_TIME = SDL_GetPerformanceCounter();

	while (true) {
		// Poll events to the input object
		SDL_Event event;
		this->input.begin_new_frame();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (!event.key.repeat) this->input.event_KeyDown(event);
				break;
			case SDL_KEYUP:
				if (!event.key.repeat) input.event_KeyUp(event);
				break;
			case SDL_MOUSEBUTTONDOWN:
				this->input.event_ButtonDown(event);
				break;
			case SDL_MOUSEBUTTONUP:
				this->input.event_ButtonUp(event);
				break;
			case SDL_QUIT:
				return;
			default:
				break;
			}
		}

		/// FOR TESTING TIMESCALE
		if (this->input.key_pressed(SDL_SCANCODE_V)) {
			this->timescale = 0.1;
		}
		else if (this->input.key_released(SDL_SCANCODE_V)) {
			this->timescale = 1.;
		}

		// Top-level input handling goes here
		if (this->input.key_pressed(Controls::READ->ESC)) { // Esc exits the game
			this->request_toggleEscMenu();
		}
		if (this->input.key_pressed(Controls::READ->F3)) {
			this->request_toggleF3();
		}

		// Measure frame time (in ms) and update 
		const auto CURRENT_TIME = SDL_GetPerformanceCounter();
		Milliseconds ELAPSED_TIME = (CURRENT_TIME - LAST_UPDATE_TIME) * 1000. / FREQUENCY;
		LAST_UPDATE_TIME = CURRENT_TIME;

		if (ELAPSED_TIME > performance::MAX_FRAME_TIME_MS) ELAPSED_TIME = performance::MAX_FRAME_TIME_MS;
			// fix for physics bugging out in low FPS moments
			// this means below 1000/40=25 FPS physics start to slow down 

		this->_true_time_elapsed = ELAPSED_TIME;
		
		if (!this->handle_requests()) return;

		this->update_everything(ELAPSED_TIME * this->timescale); // this is all there is to timescale mechanic
		this->draw_everything();
	}
}

bool Game::handle_requests() {
	// Handle esc menu toggle
	if (this->_requested_toggle_esc_menu) {
		Graphics::ACCESS->gui->EscMenu_toggle();
		this->paused = !this->paused;

		this->_requested_toggle_esc_menu = false;
	}

	// Handle F3 toggle
	if (this->_requested_toggle_F3) {
		this->toggle_F3 = !this->toggle_F3;

		this->_requested_toggle_F3 = false;
	}

	// Handle exit to desktop
	if (this->_requested_exit_to_desktop) return false;

	// Handle level change
	if (this->_requested_level_change && this->level_change_timer.finished()) {
		if (this->level_change_is_reload) {
			this->_level_loadFromSave();
		}
		else {
			this->_level_swapToTarget();
		}
	}

	return true;
}

void Game::update_everything(Milliseconds elapsedTime) {
	if (!this->paused) this->level.update(elapsedTime);

	Graphics::ACCESS->gui->update(elapsedTime);

	EmitStorage::ACCESS->update(elapsedTime);

	TimerController::ACCESS->update(elapsedTime);

	Graphics::ACCESS->camera->position = this->level.player->cameraTrap_getPosition();
}

void Game::draw_everything() {
	this->level.draw();

	// In F3 mode draw info for testing
	if (this->toggle_F3) {
		this->_drawHitboxes();
		this->_drawInfo();
	}

	Graphics::ACCESS->gui->draw();

	// Render to screen
	Graphics::ACCESS->camera->cameraToRenderer(); // draw camera content first
	Graphics::ACCESS->gui->GUIToRenderer(); // draw GUI content on top
	Graphics::ACCESS->rendererToWindow(); // apply renderer to screen

	// Clear all rendering textures
	Graphics::ACCESS->camera->cameraClear(); // clear camera backbuffer
	Graphics::ACCESS->gui->GUIClear(); // clear GUI backbuffer
	Graphics::ACCESS->rendererClear(); // clear renderer
}

// Level loading/changing
void Game::_level_swapToTarget() {
	auto extractedPlayer = this->level._extractPlayer(); // extract player

	auto playerPtr = static_cast<ntt::player::Player*>(extractedPlayer.get());

	playerPtr->position = this->level_change_position;
		// set player position in a new level,
		// this should be done before spawning in a new level

	playerPtr->cameraTrap_center();
	Graphics::ACCESS->camera->position = playerPtr->cameraTrap_getPosition();
		// center camera at the player to prevent situation where player doesn't get
		// updated due to being too far away from camera

	this->level = Level(
		this->level_change_target,
		std::move(extractedPlayer)
	); // constuct new level and transfer player to it

	this->_requested_level_change = false;

	Graphics::ACCESS->gui->AllPlayerGUI_on();

	Graphics::ACCESS->gui->Fade_on(colors::BLACK, colors::BLACK.transparent(), defaults::LEVEL_CHANGE_FADE_DURATION);
}

void Game::_level_loadFromSave() {
	const auto savedLevel = Saver::ACCESS->get_CurrentLevel();
	const auto savedPosition = Saver::ACCESS->get_PlayerPosition();

	auto constructedPlayer = std::make_unique<ntt::player::Player>(savedPosition);
		
	constructedPlayer->cameraTrap_center();
	Graphics::ACCESS->camera->position = constructedPlayer->cameraTrap_getPosition();
		// center camera at the player to prevent situation where player doesn't get
		// updated due to being too far away from camera

	this->level = Level(
		savedLevel,
		std::make_unique<ntt::player::Player>(savedPosition)
	);	

	this->_requested_level_change = false;
	this->level_change_is_reload = false;

	Graphics::ACCESS->gui->AllPlayerGUI_on();

	Graphics::ACCESS->gui->Fade_on(colors::BLACK, colors::BLACK.transparent(), defaults::LEVEL_CHANGE_FADE_DURATION);
}

// Testing
void Game::_drawHitboxes() {
	// Draw tile hitboxes and actionboxes
	SDL_Texture* tileHitboxBorder = Graphics::ACCESS->getTexture("content/textures/hitbox_border_tile.png");
	SDL_Texture* tileActionboxBorder = Graphics::ACCESS->getTexture("content/textures/actionbox_border_tile.png");

	const auto cameraPos = this->level.player->cameraTrap_getPosition();

	const Vector2 centerIndex = helpers::divide32(cameraPos);

	const int leftBound = std::max(centerIndex.x - performance::TILE_FREEZE_RANGE_X, 0);
	const int rightBound = std::min(centerIndex.x + performance::TILE_FREEZE_RANGE_X, this->level.getSizeX());
	const int upperBound = std::max(centerIndex.y - performance::TILE_FREEZE_RANGE_Y, 0);
	const int lowerBound = std::min(centerIndex.y + performance::TILE_FREEZE_RANGE_Y, this->level.getSizeY());

	// Update tiles
	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tile = this->level.getTile(X, Y);

			if (tile) {
				// Draw hibox if present
				if (tile->hitbox)
					for (const auto& hitboxRect : tile->hitbox->rectangles) {
						const dstRect destRect = hitboxRect.rect.to_dstRect();
						Graphics::ACCESS->camera->textureToCamera(tileHitboxBorder, NULL, &destRect);
					}
				// Draw actionbox if present
				if (tile->interaction) {
					const dstRect destRect = tile->interaction->actionbox.to_dstRect();
					Graphics::ACCESS->camera->textureToCamera(tileActionboxBorder, NULL, &destRect);
				}
			}
		}

	// Draw entity hitboxes
	SDL_Texture* entityHitboxBorder = Graphics::ACCESS->getTexture("content/textures/hitbox_border_entity.png");

	for (const auto &entity : this->level.entities_solid) {
		const dstRect destRect = entity->solid->getHitbox().to_dstRect();
		Graphics::ACCESS->camera->textureToCamera(entityHitboxBorder, NULL, &destRect);
	}
}

void Game::_drawInfo() const {
	// Bypasses GUI text system and draws text directly through font

	Font* const font = Graphics::ACCESS->gui->fonts.at("BLOCKY").get();
	constexpr auto gap = Vector2d(2., 2.);
	constexpr double gapX = 80.;
	constexpr double gapY = 8.;

	// TEMP
	const auto &player = this->level.player;
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