#include "gui.h"

#include "graphics.h" // access to rendering
#include "globalconsts.hpp" // natural consts
#include "item_base.h" // 'Inventory' and 'Item' classes (inventory GUI)
#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "saver.h" // checking wheter save exists upon main menu startup



// # Font #
Font::Font(SDL_Texture* texture, const Vector2 &size, const Vector2d &gap) :
	font_texture(texture),
	font_size(size),
	font_gap(gap)
{}

Vector2d Font::draw_symbol(const Vector2d &position, char symbol, bool overlay) const {
	Vector2 source_pos;

	// Letters
	if ('a' <= symbol && symbol <= 'z') { // lower case
		source_pos.set(symbol - 'a', 0);
	}
	else if ('A' <= symbol && symbol <= 'Z') { // upper case
		source_pos.set(symbol - 'A', 0);
	}
	// Space
	else if (symbol == ' ') {
		source_pos.set(0, 2);
	}
	// Numbers
	else if ('0' <= symbol && symbol <= '9') {
		source_pos.set(symbol - '0', 1);
	}
	// Symbols
	else if (symbol == ',') { // 
		source_pos.set(1, 2);
	}
	else if (symbol == '.') { // 
		source_pos.set(2, 2);
	}
	else if (symbol == '!') { // 
		source_pos.set(3, 2);
	}
	else if (symbol == '?') { // 
		source_pos.set(4, 2);
	}
	else if (symbol == '-') { // 
		source_pos.set(5, 2);
	}
	else if (symbol == ':') { // 
		source_pos.set(6, 2);
	}
	// Unknown symbol
	else {
		source_pos.set(0, 2);
	}

	srcRect sourceRect = {
		this->font_size.x * source_pos.x,
		this->font_size.y * source_pos.y,
		this->font_size.x,
		this->font_size.y
	};
	dstRect destRect = {
		position.x,
		position.y,
		static_cast<double>(this->font_size.x),
		static_cast<double>(this->font_size.y)
	};

	if (overlay) { Graphics::ACCESS->gui->textureToGUI(this->font_texture, &sourceRect, &destRect); }
	else { Graphics::ACCESS->camera->textureToCamera(this->font_texture, &sourceRect, &destRect); }

	return position + Vector2d(this->font_gap.x + this->font_size.x, 0);
}

Vector2d Font::draw_line(const Vector2d &position, const std::string &line, bool overlay) const {
	Vector2d cursor = position;
	for (const auto &letter : line) { cursor = this->draw_symbol(cursor, letter, overlay); }
	return cursor;
}

void Font::color_set(const RGBColor &color) {
	SDL_SetTextureColorMod(this->font_texture, color.r, color.g, color.b);
	SDL_SetTextureAlphaMod(this->font_texture, color.alpha);
}
void Font::color_reset() {
	SDL_SetTextureColorMod(this->font_texture, 255, 255, 255);
	SDL_SetTextureAlphaMod(this->font_texture, 255);
}

Vector2 Font::get_font_size() const {
	return this->font_size;
}
Vector2d Font::get_font_gap() const {
	return this->font_gap;
}
Vector2d Font::get_font_monospace() const {
	return this->font_gap + this->font_size;
}



// # Text #
Text::Text(const std::string &content, const dRect &bounds, Font* font) :
	content(content + ' ' ),
	bounds(bounds),
	font(font),
	finish(this->content.end())
{
	this->setup_line_breaks(); // a little clutch
}


void Text::update(Milliseconds elapsedTime) {
	if (this->delay && this->finish != this->content.end()) {
		this->time_elapsed += elapsedTime;

		// Advance 'finish' if necessary
		if (this->time_elapsed > this->delay) { // account for timescale
			this->time_elapsed = 0;
			++this->finish;
		}
	}
}

void Text::draw() const {
	this->font->color_set(this->color);

	const Vector2d fontMonospace = this->font->get_font_monospace();

	Vector2d cursor = this->bounds.getCornerTopLeft();

	std::string line = "";

	for (auto iter = this->content.begin(); iter != this->finish; ++iter) {
		line += *iter;

		if (this->line_breaks.count(iter) || iter == this->finish - 1) { // break line
			Vector2d linePos = cursor;
			if (this->centered) { linePos.x = this->bounds.getCenter().x; }

			cursor.y += fontMonospace.y;

			this->draw_line(linePos, line);

			line = "";
		}
	}
}


void Text::set_properties(const RGBColor &color, bool overlay, bool centered, Milliseconds delay) {
	this->set_color(color);
	this->set_overlay(overlay);
	this->set_centered(centered);
	this->set_delay(delay);
}

void Text::set_color(const RGBColor &color) { this->color = color; }
void Text::set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) { this->color = RGBColor(r, g, b, alpha); }
void Text::set_overlay(bool value) { this->overlay = value; }
void Text::set_centered(bool value) { this->centered = value; }
void Text::set_delay(Milliseconds delay) {
	if (delay) {
		this->delay = delay;
		this->finish = this->content.begin();
	}
}

bool Text::is_finished() const {
	return (!this->delay) || (this->finish == this->content.end());
}
const Font& Text::get_font() const {
	return *(this->font);
}

void Text::draw_line(const Vector2d &position, const std::string &line) const {
	Vector2d cursor;

	if (this->centered) {
		cursor = Vector2d(
			position.x - line.length() * this->font->get_font_monospace().x / 2.,
			position.y
		);
	}
	else {
		cursor = position;
	}

	for (const auto &letter : line) {
		cursor = this->font->draw_symbol(cursor, letter, this->overlay);
	}
}

void Text::setup_line_breaks() {
	std::string line = "";
	std::string word = "";

	Vector2d cursor = this->bounds.getCornerTopLeft();
	Vector2d checker = cursor;

	const Vector2d letter_size = this->font->get_font_monospace();

	std::string::const_iterator last_space = this->content.begin();

	for (auto iter = this->content.begin(); iter != this->content.end(); ++iter) {
		char letter = *iter;

		word += letter;
		checker.x += letter_size.x;

		// if word has ended (word also can include ,.!? etc at the end)
		if (letter == ' ') {
			// new line
			if (checker.x > this->bounds.getRight()) {

				this->line_breaks.insert(last_space);

				line = "";
				line += word;

				cursor.x = this->bounds.getLeft();
				cursor.y += letter_size.y;
				checker = cursor + Vector2d(word.length() * letter_size.x, 0);
			}
			// continue
			else {
				last_space = iter;
				line += word;

				// special behaviour if this is the last line
				if (iter == this->content.end() - 1) {
					this->line_breaks.insert(last_space);
				}
			}

			word = "";
		}
	}
}



// # GUI_FPSCounter #
GUI_FPSCounter::GUI_FPSCounter() :
	time_elapsed(0),
	frames_elapsed(0),
	currentFPS(0)
{}

void GUI_FPSCounter::update(Milliseconds elapsedTime) {
	this->time_elapsed += Game::READ->_true_time_elapsed; // FPS is calculated independent from tilecalse!
	++this->frames_elapsed;

	if (this->time_elapsed > this->UPDATE_RATE) {
		this->currentFPS = this->frames_elapsed;

		this->time_elapsed = 0;
		this->frames_elapsed = 0;

		text_handle.erase();
		this->text_handle = Graphics::ACCESS->gui->make_line(std::to_string(this->currentFPS), this->position);
		this->text_handle.get().set_color(0, 0, 0); // black
	}
}

void GUI_FPSCounter::draw() const {
	// empty, text drawing is handled by GUI object
}



// # GUI_Button #
GUI_Button::GUI_Button(const dRect& buttonRect, const std::string& displayedText, Font* font,
	const RGBColor& color, const RGBColor& colorHovered, const RGBColor& colorPressed) :
	button_rect(buttonRect),
	button_hovered_over(false),
	button_is_being_pressed(false),
	button_was_pressed(false),
	text_pos(
		buttonRect.getCenterX() - font->get_font_monospace().x / 2. * displayedText.length(),
		buttonRect.getCenterY() - font->get_font_monospace().y / 2.
	),
	displayed_text(displayedText),
	font(font),
	color(color),
	color_hovered(colorHovered),
	color_pressed(colorPressed)	
{}

void GUI_Button::update(Milliseconds elapsedTime) {
	auto &input = Game::ACCESS->input;

	const bool mouseHover = this->button_rect.containsPoint(input.mousePosition());

	if (mouseHover && this->button_hovered_over == false)
		Game::ACCESS->play_sound("gui_click.wav", 0.8); // if mouse just started hovering over play sound

	this->button_hovered_over = mouseHover;

	if (this->button_hovered_over) {
		if (input.mouse_pressed(Controls::READ->LMB)) {
			Game::ACCESS->play_sound("gui_click.wav");
			this->button_is_being_pressed = true;
		}
		else if (input.mouse_released(Controls::READ->LMB)) {
			this->button_is_being_pressed = false;
			this->button_was_pressed = true;	
		}
	}
}

void GUI_Button::draw() {
	this->font->color_set(
		this->button_is_being_pressed ? this->color_pressed :
		this->button_hovered_over ? this->color_hovered :
		this->color
	);

	this->font->draw_line(this->text_pos, this->displayed_text, true);
}

bool GUI_Button::was_pressed() const {
	return this->button_was_pressed;
}

void GUI_Button::reset() {
	this->button_was_pressed = false;
}



// # GUI_EscMenu $
namespace EscMenu_consts {
	constexpr double CENTER_X = natural::WIDTH / 2.;
	constexpr double TOP_Y = 140.;

	constexpr double BUTTON_PADDING_X = 8.; // buttons actual height is larget than just the height of the text
	constexpr double BUTTON_PADDING_Y = 12.;

	constexpr double GAP_BETWEEN_BUTTONS = 1.;

	constexpr auto COLOR_TEXT = colors::SH_YELLOW;
	constexpr auto COLOR_TEXT_HOVERED = colors::SH_YELLOW * 0.7 + colors::SH_BLACK * 0.3;
	constexpr auto COLOR_TEXT_PRESSED = colors::SH_YELLOW * 0.5 + colors::SH_BLACK * 0.5;
	constexpr auto COLOR_FADE = colors::ESC_MENU_FADE_COLOR;

	const std::string TEXT_RESUME = "resume";
	const std::string TEXT_CONTROLS = "controls";
	const std::string TEXT_START_FROM_CHECKPOINT = "restart";
	const std::string TEXT_RETURN_TO_MENU = "return to main menu";
	const std::string TEXT_EXIT_TO_DESKTOP = "exit to desktop";
}

GUI_EscMenu::GUI_EscMenu(Font* font) :
	font(font)
{
	using namespace EscMenu_consts;

	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * std::max({
		TEXT_RESUME.length(),
		TEXT_CONTROLS.length(),
		TEXT_START_FROM_CHECKPOINT.length(),
		TEXT_RETURN_TO_MENU.length(),
		TEXT_EXIT_TO_DESKTOP.length()
		});

	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	double cursorY = TOP_Y;

	this->button_resume = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_RESUME,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_controls = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_CONTROLS,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_start_from_checkpoint = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_START_FROM_CHECKPOINT,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_return_to_main_menu = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_RETURN_TO_MENU,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_exit_to_desktop = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_EXIT_TO_DESKTOP,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;
}

void GUI_EscMenu::update(Milliseconds elapsedTime) {
	if (this->button_resume) {
		this->button_resume->update(elapsedTime);

		// Handle button press
		if (this->button_resume->was_pressed())
			Game::ACCESS->request_toggleEscMenu();
	}

	if (this->button_controls) {
		this->button_controls->update(elapsedTime);

		/// NOT IMPLEMENTED
	}

	if (this->button_start_from_checkpoint) {
		this->button_start_from_checkpoint->update(elapsedTime);

		// Handle button press
		if (this->button_start_from_checkpoint->was_pressed()) {
			Game::ACCESS->request_toggleEscMenu();
			Game::ACCESS->level->player->health->instakill();
		}	
	}

	if (this->button_return_to_main_menu) { 
		this->button_return_to_main_menu->update(elapsedTime);

		if (this->button_return_to_main_menu->was_pressed()) {
			Game::ACCESS->request_toggleEscMenu();			
			Game::ACCESS->request_goToMainMenu();
		}
	}

	if (this->button_exit_to_desktop) {
		this->button_exit_to_desktop->update(elapsedTime);

		// Handle button press
		if (this->button_exit_to_desktop->was_pressed())
			Game::ACCESS->request_exitToDesktop();
	}

}

void GUI_EscMenu::draw() const {
	if (this->button_resume) this->button_resume->draw();
	if (this->button_controls) this->button_controls->draw();
	if (this->button_start_from_checkpoint) this->button_start_from_checkpoint->draw();
	if (this->button_return_to_main_menu) this->button_return_to_main_menu->draw();
	if (this->button_exit_to_desktop) this->button_exit_to_desktop->draw();
}



// # GUI_MainMenu #
namespace MainMenu_consts {
	constexpr double CENTER_X = natural::WIDTH / 2.;
	constexpr double TOP_Y = 150.;

	constexpr double BUTTON_PADDING_X = 8.; // buttons actual height is larget than just the height of the text
	constexpr double BUTTON_PADDING_Y = 12.;

	constexpr double GAP_BETWEEN_BUTTONS = 1.;

	constexpr auto COLOR_TEXT = colors::SH_YELLOW;
	constexpr auto COLOR_TEXT_HOVERED = colors::SH_YELLOW * 0.7 + colors::SH_BLACK * 0.3;
	constexpr auto COLOR_TEXT_PRESSED = colors::SH_YELLOW * 0.5 + colors::SH_BLACK * 0.5;

	const std::string TEXT_CONTINUE = "continue";
	const std::string TEXT_NEW_GAME = "new game";
	const std::string TEXT_SETTINGS = "settings";
	const std::string TEXT_EXIT = "exit";
}

GUI_MainMenu::GUI_MainMenu(Font* font) :
	font(font)
{
	this->texture = Graphics::ACCESS->getTexture_GUI("main_menu_background.png");

	using namespace MainMenu_consts;

	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * std::max({
		TEXT_CONTINUE.length(),
		TEXT_NEW_GAME.length(),
		TEXT_SETTINGS.length(),
		TEXT_EXIT.length()
		});

	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	double cursorY = TOP_Y;

	// Show 'continue' button if game save already exists
	if (Saver::READ->save_present()) {
		this->button_continue = std::make_unique<GUI_Button>(
			dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
			TEXT_CONTINUE,
			this->font,
			COLOR_TEXT,
			COLOR_TEXT_HOVERED,
			COLOR_TEXT_PRESSED
			);
		cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;
	}

	this->button_new_game = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_NEW_GAME,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_settings = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_SETTINGS,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	this->button_exit = std::make_unique<GUI_Button>(
		dRect(CENTER_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_EXIT,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;
}

void GUI_MainMenu::update(Milliseconds elapsedTime) {
	if (this->button_continue) {
		this->button_continue->update(elapsedTime);

		// Handle button press
		if (this->button_continue->was_pressed()) {
			Game::ACCESS->request_levelLoadFromSave();
		}
	}

	if (this->button_new_game) {
		this->button_new_game->update(elapsedTime);

		// Handle button press
		if (this->button_new_game->was_pressed()) {
			Saver::ACCESS->create_new();
			Game::ACCESS->request_levelLoadFromSave();
		}
	}

	if (this->button_settings) {
		this->button_settings->update(elapsedTime);

		/// TO BE IMPLEMENTED
	}

	if (this->button_exit) {
		this->button_exit->update(elapsedTime);

		// Handle button press
		if (this->button_exit->was_pressed())
			Game::ACCESS->request_exitToDesktop();
	}
}

void GUI_MainMenu::draw() const {
	Graphics::ACCESS->gui->textureToGUI(this->texture, NULL, NULL);

	if (this->button_continue) this->button_continue->draw();
	if (this->button_new_game) this->button_new_game->draw();
	if (this->button_settings) this->button_settings->draw();
	if (this->button_exit) this->button_exit->draw();
}



// # GUI_Inventory #
GUI_Inventory::GUI_Inventory(Font* font) :
	font(font)
{}

void GUI_Inventory::update(Milliseconds elapsedTime) {}

void GUI_Inventory::draw() const {
	// Formatting consts
	constexpr int ROWS = 6;
	constexpr int COLUMNS = 4;

	constexpr double ICON_SIZE = 16.;
	constexpr double TEXT_ICON_GAP = 2.; // gap between item icon and item quantity

	constexpr double FONT_WIDTH = 6.; // includes font gaps
	constexpr double FONT_HEIGHT = 5.;

	constexpr double GRID_GAP_X = 2.;
	constexpr double GRID_GAP_Y = 2.;

	constexpr auto GRID_MONOSPACE = Vector2d(ICON_SIZE + GRID_GAP_X, ICON_SIZE + TEXT_ICON_GAP + FONT_HEIGHT + GRID_GAP_Y);
	
	// Deduce size of the entire grid
	constexpr auto GRID_SIZE = Vector2d(GRID_MONOSPACE.x * COLUMNS, GRID_MONOSPACE.y * ROWS);

	// Center grid precisely at the center of the screen
	constexpr auto GRID_CORNER = Vector2d(
		natural::WIDTH / 2. - GRID_SIZE.x / 2. ,
		natural::HEIGHT / 2. - GRID_SIZE.y / 2.
	);

	// Position 'Inventory' text above the grid
	constexpr double TEXT_GRID_GAP = 4.; // gap between 'Inventory' text and grid itself
	constexpr auto TEXT_CORNER = Vector2d(
		natural::WIDTH / 2. - FONT_WIDTH * 9. / 2., // '9' is the amount of letters in a word 'Inventory'
		GRID_CORNER.y - TEXT_GRID_GAP - FONT_HEIGHT
	);

	// Draw text
	this->font->color_set(colors::FULL_BLACK);

	this->font->draw_line(TEXT_CORNER, "Inventory");

	// Draw grid of items from player inventory
	const auto &inventory = Game::ACCESS->level->player->inventory;

	Vector2d cursor = GRID_CORNER;

	for (const auto& stack : inventory.stacks) {
		// Move cursor to the next line if out of bounds
		if (cursor.x >= GRID_CORNER.x + GRID_SIZE.x) {
			cursor.x = GRID_CORNER.x;
			cursor.y += GRID_MONOSPACE.y;
		}

		// Draw item icon at cursor
		stack.item().drawAt(cursor);

		// Draw item quantity below the icon
		const std::string quantityString = std::to_string(stack.quantity());
		const double textWidth = quantityString.length() * FONT_WIDTH;

		font->draw_line(
			cursor + Vector2d(ICON_SIZE / 2. - textWidth / 2., ICON_SIZE + TEXT_ICON_GAP),
			quantityString
		);

		cursor.x += GRID_MONOSPACE.x;
	}
}

// # GUI_PlayerHealthbar #
GUI_PlayerHealthbar::GUI_PlayerHealthbar() :
	texture(Graphics::ACCESS->getTexture_GUI("player_healthbar.png")),
	percentage(1.)
{}

void GUI_PlayerHealthbar::update(Milliseconds elapsedTime) {
	this->percentage = Game::READ->level->player->health->percentage();
}

void GUI_PlayerHealthbar::draw() const {
	// Position of healthbar on the screen
	constexpr double HEALTHBAR_LEFT = 2.;
	constexpr double HEALTHBAR_BOTTOM = natural::HEIGHT - 14.;

	// Source rects on the texture
	constexpr auto BORDER_CORNER = Vector2(0, 0);
	constexpr auto BORDER_SIZE = Vector2(16, 82);

	constexpr auto FILL_CORNER = Vector2(17, 1);
	constexpr auto FILL_SIZE = Vector2(14, 80);

	constexpr auto FILL_ALIGMENT = Vector2d(1., 1.); // aligment of fill corner relative to border on the screen

	constexpr srcRect BORDER_SOURCE_RECT = {
		BORDER_CORNER.x,
		BORDER_CORNER.y,
		BORDER_SIZE.x,
		BORDER_SIZE.y
	};
	
	const int fillDisplayedHeight = static_cast<int>(FILL_SIZE.y * this->percentage); 

	const srcRect fillSourceRect = {
		FILL_CORNER.x,
		FILL_CORNER.y + FILL_SIZE.y - fillDisplayedHeight,
		FILL_SIZE.x,
		fillDisplayedHeight
	};

	// Dest rects
	constexpr dstRect BORDER_DEST_RECT = {
		HEALTHBAR_LEFT,
		HEALTHBAR_BOTTOM - BORDER_SIZE.y,
		BORDER_SIZE.x,
		BORDER_SIZE.y
	};

	const dstRect fillDestRect = {
		BORDER_DEST_RECT.x + FILL_ALIGMENT.x,
		BORDER_DEST_RECT.y + FILL_ALIGMENT.y + FILL_SIZE.y - fillDisplayedHeight,
		static_cast<double>(FILL_SIZE.x),
		static_cast<double>(fillDisplayedHeight)
	};

	// Draw
	Graphics::ACCESS->gui->textureToGUI(this->texture, &fillSourceRect, &fillDestRect);
	Graphics::ACCESS->gui->textureToGUI(this->texture, &BORDER_SOURCE_RECT, &BORDER_DEST_RECT);
}



// # GUI_CDbar #
GUI_CDbar::GUI_CDbar() {
	this->texture_border = Graphics::ACCESS->getTexture_GUI("cdbar_border.png");
	this->texture_fill = Graphics::ACCESS->getTexture_GUI("cdbar_fill.png");
}

void GUI_CDbar::update(Milliseconds elapsedTime) {
	/// CHECK PLAYER CD PERCENTAGE
	///this->percentage = Game::READ->level->player->health->percentage(); /// TEMP
	///const Timer &cooldown = Game::READ->level->player->form_change_cooldown;

	///this->percentage = cooldown.finished() ? 1. : (cooldown.elapsed() / cooldown.duration());
}
void GUI_CDbar::draw() const {
	const Vector2 border_size(75, 9);
	const Vector2 fill_size(73, 9); // actual height is 2 pixels smaller but we don't care
	const Vector2 fill_visibleSize(static_cast<int>(fill_size.x * this->percentage), fill_size.y);

	const Vector2d fill_aligment(1., 0.);
	const Vector2d fill_position = this->position + fill_aligment;
	

	const srcRect sourceRect_fill = {
		0,
		0,
		fill_visibleSize.x,
		fill_visibleSize.y
	};

	const dstRect destRect_fill = {
		fill_position.x,
		fill_position.y,
		static_cast<double>(fill_visibleSize.x),
		static_cast<double>(fill_visibleSize.y)
	};

	const dstRect destRect_border = {
		this->position.x,
		this->position.y,
		static_cast<double>(border_size.x),
		static_cast<double>(border_size.y)
	};

	Graphics::ACCESS->gui->textureToGUI(this->texture_fill, &sourceRect_fill, &destRect_fill);
	Graphics::ACCESS->gui->textureToGUI(this->texture_border, NULL, &destRect_border);
}



// # GUI_PlayerPortrait
GUI_PlayerPortrait::GUI_PlayerPortrait() :
	texture(Graphics::ACCESS->getTexture_GUI("player_portrait.png"))
{
	// Set size
	int sizeX, sizeY;
	SDL_QueryTexture(this->texture, NULL, NULL, &sizeX, &sizeY);

	this->size = Vector2d(sizeX, sizeY);
}

void GUI_PlayerPortrait::update(Milliseconds elapsedTime) {}

void GUI_PlayerPortrait::draw() const {
	constexpr double PORTRAIT_LEFT = 25.;
	constexpr double PORTRAIT_BOTTOM = natural::HEIGHT - 14.;

	const dstRect destRect = {
		PORTRAIT_LEFT,
		PORTRAIT_BOTTOM - this->size.y,
		this->size.x,
		this->size.y
	};

	Graphics::ACCESS->gui->textureToGUI(this->texture, NULL, &destRect);
}



// # StaticFade #
GUI_Fade::GUI_Fade(const RGBColor &color) :
	color(color)
{
	this->texture = Graphics::ACCESS->getTexture_GUI("fade.png"); // a texture of literally white screen
}

void GUI_Fade::update(Milliseconds elapsedTime) {}
void GUI_Fade::draw() const {
	SDL_SetTextureColorMod(this->texture, this->color.r, this->color.g, this->color.b);
	SDL_SetTextureAlphaMod(this->texture, this->color.alpha);

	Graphics::ACCESS->gui->textureToGUI(this->texture, NULL, NULL);
}



// # GUI_SmoothFade #
GUI_SmoothFade::GUI_SmoothFade(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration) :
	GUI_Fade(colorStart),
	duration(duration),
	time_elapsed(0.),
	color_start(colorStart),
	color_end(colorEnd)
{}

void GUI_SmoothFade::update(Milliseconds elapsedTime) {
	if (this->time_elapsed < this->duration) {
		this->time_elapsed += elapsedTime;

		if (this->time_elapsed > this->duration) { this->time_elapsed = this->duration; }
		
		const double B = this->time_elapsed / this->duration; // beware of integer division!
		const double A = 1. - B;

		this->color = this->color_start * A + this->color_end * B;
	}
}



// # Gui #
Gui::Gui() :
	fade_override_gui(false)
{
	this->backbuffer = SDL_CreateTexture(
		Graphics::ACCESS->getRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
		Graphics::READ->width(), Graphics::READ->height()
	);

	SDL_SetTextureBlendMode(this->backbuffer, SDL_BLENDMODE_BLEND); // necessary for proper blending of of transparent parts
	this->fonts["BLOCKY"] = (std::make_unique<Font>(
		Graphics::ACCESS->getTexture_GUI("font.png"),
		Vector2(5, 5),
		Vector2d(1., 2.)
		)
	);
}

Gui::~Gui() {
	SDL_DestroyTexture(this->backbuffer);
}

void Gui::update(Milliseconds elapsedTime) {
	// Handle GUI-realted inputs handling goes here
	auto &game = Game::ACCESS;
	auto &input = game->input;
	
	if (input.key_pressed(Controls::READ->ESC) && game->is_running()) {
		game->request_toggleEscMenu();
	}
	if (input.key_pressed(Controls::READ->F3) && game->is_running()) {
		game->request_toggleF3();
	}

	if (this->fade) { this->fade->update(elapsedTime); }

	///this->inventoryGUI.update(elapsedTime); // non-optional

	if (this->FPS_counter) this->FPS_counter->update(elapsedTime);
	if (this->main_menu) this->main_menu->update(elapsedTime);
	if (this->esc_menu) this->esc_menu->update(elapsedTime);
	if (Game::ACCESS->is_running() && this->inventory_menu) this->inventory_menu->update(elapsedTime);
	if (Game::ACCESS->is_running() && this->player_healthbar) this->player_healthbar->update(elapsedTime);
	///if (Game::ACCESS->is_running() && this->cdbar) this->cdbar->update(elapsedTime);
	if (Game::ACCESS->is_running() && this->player_portrait) this->player_portrait->update(elapsedTime);

	for (auto &text : this->texts) text.update(elapsedTime);
}

void Gui::draw() const {
	if (this->fade && !this->fade_override_gui) this->fade->draw(); // if fade doesn't override GUI

	///this->inventoryGUI.draw(); // non-optional

	if (this->FPS_counter) this->FPS_counter->draw();
	if (this->main_menu) this->main_menu->draw();
	if (this->esc_menu) this->esc_menu->draw();
	if (Game::ACCESS->is_running() && this->inventory_menu) this->inventory_menu->draw();
	if (Game::ACCESS->is_running() && this->player_healthbar) this->player_healthbar->draw();
	///if (Game::ACCESS->is_running() && this->cdbar) this->cdbar->draw();
	if (Game::ACCESS->is_running() && this->player_portrait) this->player_portrait->draw();

	for (const auto &text : this->texts) text.draw(); // text is drawn on top of everything else

	if (this->fade && this->fade_override_gui) this->fade->draw(); // if fade overrides GUI
}


Collection<Text>::handle Gui::make_text(const std::string &text, const dRect &field) {
	Font* font = this->fonts.at("BLOCKY").get();

	return this->texts.insert(text, field, font);
}

Collection<Text>::handle Gui::make_line(const std::string &line, const Vector2d &position) {
	Font* font = this->fonts.at("BLOCKY").get();

	const Vector2d monospace = font->get_font_monospace();
	const Vector2d lineSize((line.length() + 1) * monospace.x, monospace.y); // +1 is a clutch (lookup Text constructor)

	return this->texts.insert(line, dRect(position, lineSize), font); // rectangle centers itself if necessary
}

Collection<Text>::handle Gui::make_line_centered(const std::string &line, const Vector2d &position) {
	Font* font = this->fonts.at("BLOCKY").get();

	const Vector2d monospace = font->get_font_monospace();
	const Vector2d lineSize((line.length() + 1) * monospace.x, monospace.y); // +1 is a clutch (lookup Text constructor)

	return this->texts.insert(line, dRect(position + Vector2d(monospace.x / 2., 0.), lineSize, true), font);
		// rectangle centers itself
		// + monospace.x / 2 is a clutch (lookup Text constructor)
}

// FPSCounter
void Gui::FPSCounter_on() {
	if (!this->FPS_counter) {
		this->FPS_counter = std::make_unique<GUI_FPSCounter>();
	}
}

void Gui::FPSCounter_off() {
	this->FPS_counter.reset();
}

// Main menu
void Gui::MainMenu_on() {
	if (!this->main_menu)
		this->main_menu = std::make_unique<GUI_MainMenu>(this->fonts.at("BLOCKY").get());
}

void Gui::MainMenu_off() {
	this->main_menu.reset();
}

// Esc Menu
void Gui::EscMenu_on() {
	if (!this->esc_menu) {
		// Darken the screen when in esc menu
		this->Fade_on(EscMenu_consts::COLOR_FADE, false);

		this->esc_menu = std::make_unique<GUI_EscMenu>(this->fonts.at("BLOCKY").get());
	}
}

void Gui::EscMenu_off() {
	this->Fade_off();

	this->esc_menu.reset();
}

void Gui::EscMenu_toggle() {
	if (this->esc_menu) this->EscMenu_off();
	else this->EscMenu_on();
}

// Inventory
void Gui::Inventory_on() {
	if (!this->inventory_menu)
		this->inventory_menu = std::make_unique<GUI_Inventory>(this->fonts.at("BLOCKY").get());
}

void Gui::Inventory_off() {
	this->inventory_menu.reset();
}

void Gui::Inventory_toggle() {
	if (this->inventory_menu) this->Inventory_off();
	else this->Inventory_on();
}

// Player healthbar
void Gui::PlayerHealthbar_on() {
	if (!this->player_healthbar)
		this->player_healthbar = std::make_unique<GUI_PlayerHealthbar>();
}
void Gui::PlayerHealthbar_off() {
	this->player_healthbar.reset();
}

// CDbar
void Gui::CDbar_on() {
	if (!this->cdbar)
		this->cdbar = std::make_unique<GUI_CDbar>();
}
void Gui::CDbar_off() {
	this->cdbar.reset();
}

// Portrait
void Gui::Portrait_on() {
	if (!this->player_portrait)
		this->player_portrait = std::make_unique<GUI_PlayerPortrait>();
}
void Gui::Portrait_off() {
	this->player_portrait.reset();
}

void Gui::AllPlayerGUI_on() {
	this->PlayerHealthbar_on();
	this->CDbar_on();
	this->Portrait_on();
}
void Gui::AllPlayerGUI_off() {
	this->PlayerHealthbar_off();
	this->CDbar_off();
	this->Portrait_off();
}

// Fade
void Gui::Fade_on(const RGBColor &color, bool overrideGUI) {
	this->fade_override_gui = overrideGUI;

	if (this->fade) this->Fade_off(); // new fade replaces existing fade

	this->fade = std::make_unique<GUI_Fade>(color);
}
void Gui::Fade_on(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration, bool overrideGUI) {
	this->fade_override_gui = overrideGUI;

	if (this->fade) this->Fade_off(); // new fade replaces existing fade

	this->fade = std::make_unique<GUI_SmoothFade>(colorStart, colorEnd, duration);
}
void Gui::Fade_off() {
	this->fade.reset();
}


void Gui::textureToGUI(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // take target for rendering

	if (destRect) {
		SDL_Rect scaledDestRect = {
			static_cast<int>(destRect->x * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->y * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->w * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->h * Graphics::READ->scaling_factor())
		};

		SDL_RenderCopy(Graphics::ACCESS->getRenderer(), texture, sourceRect, &scaledDestRect);
	}
	else { // (destRect == NULL) indicates that (destRect == <the entire renderer>)
		SDL_RenderCopy(Graphics::ACCESS->getRenderer(), texture, sourceRect, NULL);
	}
	
}
void Gui::textureToGUIEx(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect, double angle, SDL_RendererFlip flip) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // take target for rendering

	if (destRect) {
		SDL_Rect scaledDestRect = {
			static_cast<int>(destRect->x * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->y * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->w * Graphics::READ->scaling_factor()),
			static_cast<int>(destRect->h * Graphics::READ->scaling_factor())
		};

		SDL_RenderCopyEx(Graphics::ACCESS->getRenderer(), texture, sourceRect, &scaledDestRect, angle, NULL, flip);
	}
	else { // (destRect == NULL) indicates that (destRect == <the entire renderer>)
		SDL_RenderCopyEx(Graphics::ACCESS->getRenderer(), texture, sourceRect, NULL, angle, NULL, flip);
	}
	
}

void Gui::GUIToRenderer() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // give target back to the renderer

	Graphics::ACCESS->copyTextureToRenderer(this->backbuffer, NULL, NULL);
}
void Gui::GUIClear() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // take target for rendering

	SDL_RenderClear(Graphics::ACCESS->getRenderer());
}