#include "gui.h"

#include <iostream> // console output

#include "graphics.h" // access to rendering
#include "globalconsts.hpp" // natural consts
#include "item_base.h" // 'Inventory' and 'Item' classes (inventory GUI)
#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "saver.h" // checking wheter save exists upon main menu startup



// # Font #
Font::Font(sf::Texture* texture, const Vector2 &size, const Vector2d &gap) :
	font_size(size),
	font_gap(gap)
{
	this->sprite.setTexture(*texture);
}

Vector2d Font::draw_symbol(const Vector2d &position, char symbol, bool overlay) {
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
	else if (symbol == ',') {
		source_pos.set(1, 2);
	}
	else if (symbol == '.') {
		source_pos.set(2, 2);
	}
	else if (symbol == '!') { 
		source_pos.set(3, 2);
	}
	else if (symbol == '?') {
		source_pos.set(4, 2);
	}
	else if (symbol == '-') {
		source_pos.set(5, 2);
	}
	else if (symbol == ':') {
		source_pos.set(6, 2);
	}
	else if (symbol == '(') {
		source_pos.set(7, 2);
	}
	else if (symbol == ')') {
		source_pos.set(8, 2);
	}
	else if (symbol == '%') {
		source_pos.set(9, 2);
	}
	else if (symbol == '+') {
		source_pos.set(10, 2);
	}
	// Unknown symbol
	else {
		source_pos.set(0, 2);
	}
	
	this->sprite.setTextureRect(sf::IntRect(
		(this->font_size.x + 2) * source_pos.x,
		(this->font_size.y + 2) * source_pos.y,
		this->font_size.x + 2,
		this->font_size.y + 2
	));

	this->sprite.setPosition(
		static_cast<float>(position.x - 1),
		static_cast<float>(position.y - 1)
	);
		
	// > No need to scale
	
	if (overlay) { Graphics::ACCESS->gui->draw_sprite(sprite); }
	else { Graphics::ACCESS->camera->draw_sprite(sprite); }

	// Advance cursor with consideration to font scale
	const double scale = static_cast<double>(this->sprite.getScale().x);

	return position + Vector2d(
		(this->font_gap.x + this->font_size.x) * scale,
		0
	);
}

Vector2d Font::draw_line(const Vector2d &position, const std::string &line, bool overlay) {
	Vector2d cursor = position;
	for (const auto &letter : line) { cursor = this->draw_symbol(cursor, letter, overlay); }
	return cursor;
}

void Font::draw_line_centered(const Vector2d &position, const std::string &line, bool overlay) {
	const Vector2d corner_position_after_centering(
		position.x - this->get_font_monospace().x / 2. * this->sprite.getScale().x * line.length(),
		position.y - this->get_font_monospace().y / 2. * this->sprite.getScale().y
	);

	draw_line(corner_position_after_centering, line, overlay);
}

void Font::color_set(const RGBColor &color) {
	this->sprite.setColor(sf::Color(color.r, color.g, color.b, color.alpha));
}
void Font::color_reset() {
	this->sprite.setColor(sf::Color(255, 255, 255, 255));
}

void Font::scale_set(double scale) {
	this->sprite.setScale(static_cast<float>(scale), static_cast<float>(scale));
}
void Font::scale_reset() {
	this->sprite.setScale(1.f, 1.f);
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
	content(content + ' '),
	bounds(bounds),
	font(font),
	finish(this->content.end()),
	scale(1)
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
	this->font->scale_set(this->scale);

	const Vector2d fontMonospace = this->font->get_font_monospace() * scale;

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

	this->font->scale_reset(); // clean up changes in font
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
void Text::set_scale(double scale) {
	// set scale
	this->scale = scale;

	// resize bounds
	const auto center = this->bounds.getCenter();
	const auto size = this->bounds.getSize();

	this->bounds = dRect(center, size * scale, true);
}

bool Text::is_finished() const {
	return (!this->delay) || (this->finish == this->content.end());
}
const Font& Text::get_font() const {
	return *(this->font);
}

void Text::draw_line(const Vector2d &position, const std::string &line) const {
	Vector2d cursor;

	const Vector2d fontMonospace = this->font->get_font_monospace() * this->scale;

	if (this->centered) {
		cursor = Vector2d(
			position.x - line.length() * fontMonospace.x / 2.,
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
namespace GUI_FPSCounter_consts {
	constexpr RGBColor TEXT_COLOR = colors::FULL_BLACK;
	constexpr Vector2d POSITION = Vector2d(2., 352.);
	constexpr Milliseconds UPDATE_RATE = sec_to_ms(1.);
}

GUI_FPSCounter::GUI_FPSCounter(Font* font) :
	font(font),
	time_elapsed(0),
	frames_elapsed(0),
	currentFPS(0)
{}

void GUI_FPSCounter::update(Milliseconds elapsedTime) {
	using namespace GUI_FPSCounter_consts;

	this->time_elapsed += Game::READ->_true_time_elapsed; // FPS is calculated independent from tilecalse!
	++this->frames_elapsed;

	if (this->time_elapsed > UPDATE_RATE) {
		this->currentFPS = this->frames_elapsed;

		this->time_elapsed = 0;
		this->frames_elapsed = 0;
	}

	//this->time_elapsed += Game::READ->_true_time_elapsed; // FPS is calculated independent from tilecalse!
	//++this->frames_elapsed;

	//if (this->time_elapsed > this->UPDATE_RATE) {
	//	this->currentFPS = this->frames_elapsed;

	//	this->time_elapsed = 0;
	//	this->frames_elapsed = 0;

	//	text_handle.erase();
	//	this->text_handle = Graphics::ACCESS->gui->make_line(std::to_string(this->currentFPS), this->position);
	//	this->text_handle.get().set_color(0, 0, 0); // black
	//}
}

void GUI_FPSCounter::draw() const {
	using namespace GUI_FPSCounter_consts;

	this->font->color_set(TEXT_COLOR);
	this->font->draw_line(POSITION, std::to_string(this->currentFPS));
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

	// 'Main' tab
	const std::string TEXT_RESUME = "resume";
	const std::string TEXT_CONTROLS = "controls";
	const std::string TEXT_START_FROM_CHECKPOINT = "restart";
	const std::string TEXT_RETURN_TO_MENU = "return to main menu";
	const std::string TEXT_EXIT_TO_DESKTOP = "exit to desktop";

	// 'Controls' tab
	constexpr double LEFT_X = natural::WIDTH * (0.5 - 0.1);
	constexpr double RIGHT_X = natural::WIDTH * (0.5 + 0.1);

	const std::string TEXT_BACK = "back";

	const std::string TEXT_TABCONTROLS_ATTACK = "attack";
	const std::string TEXT_TABCONTROLS_SKILL = "skill";
	const std::string TEXT_TABCONTROLS_CHARGE_JUMP = "charge jump";
	const std::string TEXT_TABCONTROLS_JUMP = "jump";
	const std::string TEXT_TABCONTROLS_INTERACT = "interact";
	const std::string TEXT_TABCONTROLS_INVENTORY = "inventory";
}

GUI_EscMenu::GUI_EscMenu(Font* font) :
	font(font),
	current_tab(Tab::MAIN)
{
	using namespace EscMenu_consts;

	// Set up offsets that depend on font
	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * std::max({
		TEXT_RESUME.length(),
		TEXT_CONTROLS.length(),
		TEXT_START_FROM_CHECKPOINT.length(),
		TEXT_RETURN_TO_MENU.length(),
		TEXT_EXIT_TO_DESKTOP.length()
		});

	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	// Build elements of the 'main' tab
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

	// Build elements of the 'controls' tab
	this->button_back = std::make_unique<GUI_Button>(
		dRect(CENTER_X, TOP_Y + 6 * (buttonHeight + GAP_BETWEEN_BUTTONS), buttonWidth, buttonHeight, true),
		TEXT_BACK,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
}

void GUI_EscMenu::update(Milliseconds elapsedTime) {
	// Select different update_<tabname>() base on current tab
	switch (this->current_tab) {
	case Tab::MAIN:
		this->update_tab_main(elapsedTime);
		break;
	case Tab::CONTROLS:
		this->update_tab_controls(elapsedTime);
		break;
	default:
		// should be unreachable
		break;
	}
}

void GUI_EscMenu::update_tab_main(Milliseconds elapsedTime) {
	if (this->button_resume) {
		this->button_resume->update(elapsedTime);

		// Handle button press
		if (this->button_resume->was_pressed())
			Game::ACCESS->request_toggleEscMenu();
	}

	if (this->button_controls) {
		this->button_controls->update(elapsedTime);

		// Handle button press
		if (this->button_controls->was_pressed()) {
			this->button_controls->reset();
				// if we don't reset button it will still count as pressed upon going back to 'main' tab
			this->current_tab = Tab::CONTROLS;
		}
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

		// Handle button press
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

void GUI_EscMenu::update_tab_controls(Milliseconds elapsedTime) {
	if (this->button_back) {
		this->button_back->update(elapsedTime);

		// Handle button press
		if (this->button_back->was_pressed()) {
			this->button_back->reset();
				// if we don't reset button it will still count as pressed upon going back to 'controls' tab
			this->current_tab = Tab::MAIN;
		}
	}
}

void GUI_EscMenu::draw() const {
	// Select different draw_<tabname>() base on current tab
	switch (this->current_tab) {
	case Tab::MAIN:
		this->draw_tab_main();
		break;
	case Tab::CONTROLS:
		this->draw_tab_controls();
		break;
	default:
		// should be unreachable
		break;
	}
}

void GUI_EscMenu::draw_tab_main() const {
	if (this->button_resume)                this->button_resume->draw();
	if (this->button_controls)              this->button_controls->draw();
	if (this->button_start_from_checkpoint) this->button_start_from_checkpoint->draw();
	if (this->button_return_to_main_menu)   this->button_return_to_main_menu->draw();
	if (this->button_exit_to_desktop)       this->button_exit_to_desktop->draw();
}

void GUI_EscMenu::draw_tab_controls() const {
	using namespace EscMenu_consts;

	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * TEXT_BACK.length(); /// what is that for???
	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	// Draw control keys
	double cursorY = TOP_Y;

	this->font->color_set(COLOR_TEXT);

	// Attack
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_ATTACK, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->CHAIN), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Skill
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_SKILL, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->SKILL), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Charge jump
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_CHARGE_JUMP, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->SHIFT), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Jump
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_JUMP, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->JUMP), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Interact
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_INTERACT, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->USE), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Inventory
	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABCONTROLS_INVENTORY, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), toString(Controls::READ->INVENTORY), true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// 'Back' button
	if (this->button_back) this->button_back->draw();
}



// # GUI_MainMenu #
namespace MainMenu_consts {
	constexpr int TEXTURE_WIDTH = 128;
	constexpr int TEXTURE_HEIGHT = 72;

	constexpr double CENTER_X = natural::WIDTH / 2.;

	constexpr double BUTTON_PADDING_X = 8.; // buttons actual height is larget than just the height of the text
	constexpr double BUTTON_PADDING_Y = 12.;

	constexpr double GAP_BETWEEN_BUTTONS = 1.;

	constexpr auto COLOR_TEXT = colors::SH_YELLOW;
	constexpr auto COLOR_TEXT_HOVERED = colors::SH_YELLOW * 0.7 + colors::SH_BLACK * 0.3;
	constexpr auto COLOR_TEXT_PRESSED = colors::SH_YELLOW * 0.5 + colors::SH_BLACK * 0.5;

	// 'Main' tab
	constexpr double TOP_Y_MAIN = 150.;

	const std::string TEXT_CONTINUE = "continue";
	const std::string TEXT_NEW_GAME = "new game";
	const std::string TEXT_SETTINGS = "settings";
	const std::string TEXT_EXIT = "exit";

	// 'Settings' tab
	constexpr double TOP_Y_SETTINGS = 130.;
	constexpr double ADDITIONAL_BOTTOM_GAP = 4.; // additional gap between 'apply', 'cancel' and settings rows
	
	constexpr double LEFT_X = natural::WIDTH * (0.5 - 0.1);

	constexpr double RIGHT_X = natural::WIDTH * (0.5 + 0.1);
	constexpr double RIGHT_DECREASE_X = natural::WIDTH * (0.5 + 0.1 - 0.07);
	constexpr double RIGHT_INCREASE_X = natural::WIDTH * (0.5 + 0.1 + 0.07);

	const std::string TEXT_DECREASE = "(";
	const std::string TEXT_INCREASE = ")";

	const std::string TEXT_TABSETTINGS_RESOLUTION = "resolution";
	const std::vector<int> RESOLUTION_OPTIONS_X = { 640, 1280, 1920 };
	const std::vector<int> RESOLUTION_OPTIONS_Y = { 360,  720, 1080 };

	const std::string TEXT_TABSETTINGS_SCREENMODE = "screen mode";
	const std::vector<std::string> SCREENMODE_OPTIONS = { "WINDOW", "BORDERLESS", "FULLSCREEN" };

	const std::string TEXT_TABSETTINGS_MUSIC = "music";
	const std::vector<int> MUSIC_OPTIONS = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	const std::string TEXT_TABSETTINGS_SOUND = "sound";
	const std::vector<int> SOUND_OPTIONS = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	const std::string TEXT_TABSETTINGS_FPS = "fps counter";
	const std::vector<bool> FPS_OPTIONS = { true, false };

	const std::string TEXT_CANCEL = "cancel";
	const std::string TEXT_APPLY = "apply";
}

template<typename T>
int pos_in_vector(const std::vector<T> &vector, const T &value) {
	// used for setting current options accordint to parsed config

	// used for testing whether custom resolution was added or not
	// (returns negative if value wasn't found)

	for (int i = 0; i < static_cast<int>(vector.size()); ++i)
		if (vector[i] == value)
			return i;

	return -1;
}

GUI_MainMenu::GUI_MainMenu(Font* font) :
	font(font),
	current_tab(Tab::MAIN),
	config_was_changed(false),
	resolution_options_x(MainMenu_consts::RESOLUTION_OPTIONS_X),
	resolution_options_y(MainMenu_consts::RESOLUTION_OPTIONS_Y)
{
	using namespace MainMenu_consts;

	this->sprite.setTexture(Graphics::ACCESS->getTexture_GUI("main_menu_background.png"));
	this->sprite.setScale(
		static_cast<float>(natural::DIMENSIONS.x / TEXTURE_WIDTH),
		static_cast<float>(natural::DIMENSIONS.y / TEXTURE_HEIGHT)
	);

	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * std::max({
		TEXT_CONTINUE.length(),
		TEXT_NEW_GAME.length(),
		TEXT_SETTINGS.length(),
		TEXT_EXIT.length()
		});

	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	// Build elements of the 'main' tab
	double cursorY = TOP_Y_MAIN;

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

	// Build elements of the 'settings' tab
	cursorY = TOP_Y_SETTINGS;

	// Resolution
	this->resolution_decrease = std::make_unique<GUI_Button>(
		dRect(RIGHT_DECREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_DECREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	this->resolution_increase = std::make_unique<GUI_Button>(
		dRect(RIGHT_INCREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_INCREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Screen mode
	this->screenmode_decrease = std::make_unique<GUI_Button>(
		dRect(RIGHT_DECREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_DECREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	this->screenmode_increase = std::make_unique<GUI_Button>(
		dRect(RIGHT_INCREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_INCREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Music
	this->music_decrease = std::make_unique<GUI_Button>(
		dRect(RIGHT_DECREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_DECREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	this->music_increase = std::make_unique<GUI_Button>(
		dRect(RIGHT_INCREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_INCREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Sound
	this->sound_decrease = std::make_unique<GUI_Button>(
		dRect(RIGHT_DECREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_DECREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	this->sound_increase = std::make_unique<GUI_Button>(
		dRect(RIGHT_INCREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_INCREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// FPS counter
	this->fps_decrease = std::make_unique<GUI_Button>(
		dRect(RIGHT_DECREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_DECREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	this->fps_increase = std::make_unique<GUI_Button>(
		dRect(RIGHT_INCREASE_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_INCREASE,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS + ADDITIONAL_BOTTOM_GAP;

	// 'Apply' and 'Cancel'

	this->button_cancel = std::make_unique<GUI_Button>(
		dRect(LEFT_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_CANCEL,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	this->button_apply = std::make_unique<GUI_Button>(
		dRect(RIGHT_X, cursorY, buttonWidth, buttonHeight, true),
		TEXT_APPLY,
		this->font,
		COLOR_TEXT, 
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;
}

void GUI_MainMenu::update(Milliseconds elapsedTime) {
	// Select different update_<tabname>() base on current tab
	switch (this->current_tab) {
	case Tab::MAIN:
		this->update_tab_main(elapsedTime);
		break;
	case Tab::SETTINGS:
		this->update_tab_settings(elapsedTime);
		break;
	default:
		// should be unreachable
		break;
	}
}

void GUI_MainMenu::update_tab_main(Milliseconds elapsedTime) {
	using namespace MainMenu_consts;

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

		// Handle button press
		if (this->button_settings->was_pressed()) {
			this->button_settings->reset();
				// if we don't reset button it will still count as pressed upon going back to 'main' tab

			// Parse current state of config
			this->config_was_changed = false;

			int resolution_x;
			int resolution_y;
			std::string screen_mode;
			int music;
			int sound;
			bool fps_counter;
			std::string save_filepath;

			config_parse(
				resolution_x,
				resolution_y,
				screen_mode,
				music,
				sound,
				fps_counter,
				save_filepath
			);

			// Detect if custom resolution was selected, add it to options if yes
			const int resolution_x_option = pos_in_vector(this->resolution_options_x, resolution_x);
			const int resolution_y_option = pos_in_vector(this->resolution_options_y, resolution_y);

			const bool resolution_is_noncustom =
				(resolution_x_option >= 0) && (resolution_y_option >= 0) && (resolution_x_option == resolution_y_option);

			if (!resolution_is_noncustom) {
				this->resolution_options_x.push_back(resolution_x);
				this->resolution_options_y.push_back(resolution_y);
			}

			// Set selected options according to config
			this->resolution_current_option = pos_in_vector(this->resolution_options_x, resolution_x);
			this->screenmode_current_option = pos_in_vector(SCREENMODE_OPTIONS, screen_mode);
			this->music_current_option = pos_in_vector(MUSIC_OPTIONS, music);
			this->sound_current_option = pos_in_vector(SOUND_OPTIONS, sound);
			this->fps_current_option = pos_in_vector(FPS_OPTIONS, fps_counter);
			this->parsed_save_filepath = save_filepath;

			// Switch tab
			this->current_tab = Tab::SETTINGS;
		}
	}

	if (this->button_exit) {
		this->button_exit->update(elapsedTime);

		// Handle button press
		if (this->button_exit->was_pressed())
			Game::ACCESS->request_exitToDesktop();
	}
}

void GUI_MainMenu::update_tab_settings(Milliseconds elapsedTime) {
	using namespace MainMenu_consts;

	// Resolution
	if (this->resolution_decrease) {
		this->resolution_decrease->update(elapsedTime);

		// Handle button press
		if (this->resolution_decrease->was_pressed()) {
			this->resolution_decrease->reset();

			// Decrease or loop around
			this->resolution_current_option = (this->resolution_current_option - 1 >= 0)
				? (this->resolution_current_option - 1)
				: this->resolution_options_x.size() - 1;
		}
	}

	if (this->resolution_increase) {
		this->resolution_increase->update(elapsedTime);

		// Handle button press
		if (this->resolution_increase->was_pressed()) {
			this->resolution_increase->reset();

			// Increase or loop around
			this->resolution_current_option = (this->resolution_current_option + 1 <= static_cast<int>(this->resolution_options_x.size()) - 1)
				? (this->resolution_current_option + 1)
				: 0;
		}
	}

	// Screen mode
	if (this->screenmode_decrease) {
		this->screenmode_decrease->update(elapsedTime);

		// Handle button press
		if (this->screenmode_decrease->was_pressed()) {
			this->screenmode_decrease->reset();

			// Decrease or loop around
			this->screenmode_current_option = (this->screenmode_current_option - 1 >= 0)
				? (this->screenmode_current_option - 1)
				: SCREENMODE_OPTIONS.size() - 1;
		}
	}

	if (this->screenmode_increase) {
		this->screenmode_increase->update(elapsedTime);

		// Handle button press
		if (this->screenmode_increase->was_pressed()) {
			this->screenmode_increase->reset();

			// Increase or loop around
			this->screenmode_current_option = (this->screenmode_current_option + 1 <= static_cast<int>(SCREENMODE_OPTIONS.size()) - 1)
				? (this->screenmode_current_option + 1)
				: 0;
		}
	}

	// Music
	if (this->music_decrease) {
		this->music_decrease->update(elapsedTime);

		// Handle button press
		if (this->music_decrease->was_pressed()) {
			this->music_decrease->reset();

			// Decrease or loop around
			this->music_current_option = (this->music_current_option - 1 >= 0)
				? (this->music_current_option - 1)
				: MUSIC_OPTIONS.size() - 1;
		}
	}

	if (this->music_increase) {
		this->music_increase->update(elapsedTime);

		// Handle button press
		if (this->music_increase->was_pressed()) {
			this->music_increase->reset();

			// Increase or loop around
			this->music_current_option = (this->music_current_option + 1 <= static_cast<int>(MUSIC_OPTIONS.size()) - 1)
				? (this->music_current_option + 1)
				: 0;
		}
	}

	// Sound
	if (this->sound_decrease) {
		this->sound_decrease->update(elapsedTime);

		// Handle button press
		if (this->sound_decrease->was_pressed()) {
			this->sound_decrease->reset();

			// Decrease or loop around
			this->sound_current_option = (this->sound_current_option - 1 >= 0)
				? (this->sound_current_option - 1)
				: SOUND_OPTIONS.size() - 1;
		}
	}

	if (this->sound_increase) {
		this->sound_increase->update(elapsedTime);

		// Handle button press
		if (this->sound_increase->was_pressed()) {
			this->sound_increase->reset();

			// Increase or loop around
			this->sound_current_option = (this->sound_current_option + 1 <= static_cast<int>(SOUND_OPTIONS.size()) - 1)
				? (this->sound_current_option + 1)
				: 0;
		}
	}

	// FPS Counter
	if (this->fps_decrease) {
		this->fps_decrease->update(elapsedTime);

		// Handle button press
		if (this->fps_decrease->was_pressed()) {
			this->fps_decrease->reset();

			// Decrease or loop around
			this->fps_current_option = (this->fps_current_option - 1 >= 0)
				? (this->fps_current_option - 1)
				: FPS_OPTIONS.size() - 1;
		}
	}

	if (this->fps_increase) {
		this->fps_increase->update(elapsedTime);

		// Handle button press
		if (this->fps_increase->was_pressed()) {
			this->fps_increase->reset();

			// Increase or loop around
			this->fps_current_option = (this->fps_current_option + 1 <= static_cast<int>(FPS_OPTIONS.size()) - 1)
				? (this->fps_current_option + 1)
				: 0;
		}
	}

	// 'Cancel' and 'Apply'
	if (this->button_cancel) {
		this->button_cancel->update(elapsedTime);

		// Handle button press
		if (this->button_cancel->was_pressed()) {
			this->button_cancel->reset();
				// if we don't reset button it will still count as pressed upon going back to 'controls' tab
			this->current_tab = Tab::MAIN;
		}
	}

	if (this->button_apply) {
		this->button_apply->update(elapsedTime);

		// Handle button press
		if (this->button_apply->was_pressed()) {
			this->button_apply->reset();
				// not really necessary but still done for uniformity

			// Rewrite config with new parameters
			config_create(
				this->resolution_options_x[this->resolution_current_option],
				this->resolution_options_y[this->resolution_current_option],
				SCREENMODE_OPTIONS[this->screenmode_current_option],
				MUSIC_OPTIONS[this->music_current_option],
				SOUND_OPTIONS[this->sound_current_option],
				FPS_OPTIONS[this->fps_current_option],
				this->parsed_save_filepath
			);

			// Restart the game
			Game::ACCESS->request_exitToRestart();
		}
	}
}

void GUI_MainMenu::draw() {
	// Select different draw_<tabname>() base on current tab
	switch (this->current_tab) {
	case Tab::MAIN:
		this->draw_tab_main();
		break;
	case Tab::SETTINGS:
		this->draw_tab_settings();
		break;
	default:
		// should be unreachable
		break;
	}
}

void GUI_MainMenu::draw_tab_main() {
	// Background
	Graphics::ACCESS->gui->draw_sprite(this->sprite);

	// Buttons
	if (this->button_continue) this->button_continue->draw();
	if (this->button_new_game) this->button_new_game->draw();
	if (this->button_settings) this->button_settings->draw();
	if (this->button_exit) this->button_exit->draw();
}

void GUI_MainMenu::draw_tab_settings() {
	using namespace MainMenu_consts;

	const auto monospace = this->font->get_font_monospace();
	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	// Background
	Graphics::ACCESS->gui->draw_sprite(this->sprite);

	// Draw option labels
	double cursorY = TOP_Y_SETTINGS;

	this->font->color_set(COLOR_TEXT);

	// Resolution
	const std::string resolution_string =
		std::to_string(this->resolution_options_x[this->resolution_current_option]) +
		" x " +
		std::to_string(this->resolution_options_y[this->resolution_current_option]);

	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABSETTINGS_RESOLUTION, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), resolution_string, true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Screen mode
	const std::string screenmode_string = SCREENMODE_OPTIONS[this->screenmode_current_option];

	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABSETTINGS_SCREENMODE, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), screenmode_string, true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Music
	const std::string music_string = std::to_string(MUSIC_OPTIONS[this->music_current_option]);

	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABSETTINGS_MUSIC, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), music_string, true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// Sound
	const std::string sound_string = std::to_string(SOUND_OPTIONS[this->sound_current_option]);

	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABSETTINGS_SOUND, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), sound_string, true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// FPS
	const std::string fps_string = FPS_OPTIONS[this->fps_current_option] ? "on" : "off";

	this->font->draw_line_centered(Vector2d(LEFT_X, cursorY), TEXT_TABSETTINGS_FPS, true);
	this->font->draw_line_centered(Vector2d(RIGHT_X, cursorY), fps_string, true);
	cursorY += buttonHeight + GAP_BETWEEN_BUTTONS;

	// 'Decrease' and 'Increase' buttons
	if (this->resolution_decrease) this->resolution_decrease->draw();
	if (this->resolution_increase) this->resolution_increase ->draw();

	if (this->screenmode_decrease) this->screenmode_decrease->draw();
	if (this->screenmode_increase) this->screenmode_increase->draw();

	if (this->music_decrease) this->music_decrease->draw();
	if (this->music_increase) this->music_increase->draw();

	if (this->sound_decrease) this->sound_decrease->draw();
	if (this->sound_increase) this->sound_increase->draw();

	if (this->fps_decrease) this->fps_decrease->draw();
	if (this->fps_increase) this->fps_increase->draw();

	// 'Cancel' and 'Apply' buttons
	if (this->button_cancel) this->button_cancel->draw();
	if (this->button_apply) this->button_apply->draw();
}



// # GUI_EndingScreen #
namespace EndingScreen_consts {
	constexpr auto COLOR_TEXT = colors::SH_YELLOW;

	constexpr auto COLOR_TEXT_HOVERED = colors::SH_YELLOW * 0.7 + colors::SH_BLACK * 0.3;
	constexpr auto COLOR_TEXT_PRESSED = colors::SH_YELLOW * 0.5 + colors::SH_BLACK * 0.5;

	constexpr double CENTER_X = natural::WIDTH / 2.;

	constexpr double BUTTON_PADDING_X = 8.; // buttons actual height is larget than just the height of the text
	constexpr double BUTTON_PADDING_Y = 12.;

	// Text
	constexpr double TOP_Y_TEXT = 150.;

	constexpr double GAP_BETWEEN_LINES = 3.;

	const std::string TEXT_1 = "Many foes slain, many places explored.";
	const std::string TEXT_2 = "What awaits further? Only time can tell.";

	// Buttons
	constexpr double TOP_Y_BUTTONS = 250.;

	constexpr double LEFT_X = natural::WIDTH * (0.5 - 0.15);
	constexpr double RIGHT_X = natural::WIDTH * (0.5 + 0.15);

	const std::string TEXT_CONTINUE_PLAYTHROUGH = "Keep the old world";
	const std::string TEXT_FINISH_PLAYTHROUGH = "Start anew";

	// Point where player continues the playthough
	const std::string CONTINUE_PLAYTHROUGH_LEVEL = "library";
	constexpr Vector2d CONTINUE_PLAYTHROUGH_PLAYER_POS = Vector2d(1008, 1168);

	const std::string REMOVED_CHECKPOINTS_SUBSTRING = "boss_mage_checkpoint";
}

GUI_EndingScreen::GUI_EndingScreen(Font* font) :
	font(font)
{
	using namespace EndingScreen_consts;

	const auto monospace = this->font->get_font_monospace();

	const double buttonWidth = BUTTON_PADDING_X + monospace.x * std::max({
		TEXT_CONTINUE_PLAYTHROUGH.length(),
		TEXT_FINISH_PLAYTHROUGH.length()
		});

	const double buttonHeight = BUTTON_PADDING_Y + monospace.y;

	this->button_continue_playthrough = std::make_unique<GUI_Button>(
		dRect(LEFT_X, TOP_Y_BUTTONS, buttonWidth, buttonHeight, true),
		TEXT_CONTINUE_PLAYTHROUGH,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);

	this->button_finish_playthrough = std::make_unique<GUI_Button>(
		dRect(RIGHT_X, TOP_Y_BUTTONS, buttonWidth, buttonHeight, true),
		TEXT_FINISH_PLAYTHROUGH,
		this->font,
		COLOR_TEXT,
		COLOR_TEXT_HOVERED,
		COLOR_TEXT_PRESSED
		);
}

void GUI_EndingScreen::update(Milliseconds elapsedTime) {
	using namespace EndingScreen_consts;

	if (this->button_continue_playthrough) {
		this->button_continue_playthrough->update(elapsedTime);

		// Handle button press
		if (this->button_continue_playthrough->was_pressed()) {
			this->button_continue_playthrough->reset();

			// Reset bossfight checkpoints
			Flags::ACCESS->remove_containing_substring(REMOVED_CHECKPOINTS_SUBSTRING);

			// Go back to the level before boss
			Saver::ACCESS->state_set_level_and_position(CONTINUE_PLAYTHROUGH_LEVEL, CONTINUE_PLAYTHROUGH_PLAYER_POS);
			Saver::ACCESS->state_set_flags(Flags::ACCESS->flags);
			Saver::ACCESS->write();

			Game::ACCESS->request_levelLoadFromSave();
		}
	}

	if (this->button_finish_playthrough) {
		this->button_finish_playthrough->update(elapsedTime);

		// Handle button press
		if (this->button_finish_playthrough->was_pressed()) {
			this->button_finish_playthrough->reset();

			// Rename old savefile to 'restart game' while keeping the old save
			Saver::ACCESS->backup_and_delete_current();

			Game::ACCESS->request_goToMainMenu();
		}
	}
}

void GUI_EndingScreen::draw() {
	using namespace EndingScreen_consts;

	const auto monospace = this->font->get_font_monospace();

	// Text
	double cursorY = TOP_Y_TEXT;

	this->font->color_set(COLOR_TEXT);

	// Attack
	this->font->draw_line_centered(Vector2d(CENTER_X, cursorY), TEXT_1, true);
	cursorY += monospace.y + GAP_BETWEEN_LINES;

	this->font->draw_line_centered(Vector2d(CENTER_X, cursorY), TEXT_2, true);

	// Buttons
	if (this->button_continue_playthrough) this->button_continue_playthrough->draw();
	if (this->button_finish_playthrough) this->button_finish_playthrough->draw();
}



// # GUI_Inventory #
namespace GUI_Inventory_consts {
	constexpr auto TEXT_COLOR = colors::SH_YELLOW;

	constexpr double ICON_SIZE = 32;
	constexpr double FONT_HEIGHT = 5;

	constexpr Vector2d CURSOR_START = Vector2d(natural::DIMENSIONS.x * 0.15, natural::DIMENSIONS.y * 0.05);

	constexpr Vector2d OFFSET_ICON = Vector2d(0, 0);

	constexpr double ICON_TEXT_GAP = 15;

	constexpr Vector2d OFFSET_LABEL = Vector2d(ICON_SIZE + ICON_TEXT_GAP, 0);
	constexpr Vector2d OFFSET_FOUND = Vector2d(ICON_SIZE + ICON_TEXT_GAP, FONT_HEIGHT + 8);
	constexpr Vector2d OFFSET_DESCRIPTION = Vector2d(ICON_SIZE + ICON_TEXT_GAP, FONT_HEIGHT + 8 + FONT_HEIGHT + 9);

	constexpr double CELL_GAP = 15;
}

GUI_Inventory::GUI_Inventory(Font* font) :
	font(font)
{}

void GUI_Inventory::update(Milliseconds elapsedTime) {
}

void GUI_Inventory::draw() {
	using namespace GUI_Inventory_consts;

	auto &stacks = Game::ACCESS->level->player->inventory.stacks;

	Vector2d cursor = CURSOR_START;

	for (size_t i = 0; i < stacks.size(); ++i) {
		// Get displayed data from stack
		auto &item = stacks[i].item();
		const auto &quantity = stacks[i].quantity();

		const std::string label = item.getLabel();
		const std::string found = std::to_string(quantity) + " artifacts found";
		const std::string description = "effect: " + item.get_description_effect();

		// Add 'additional description' which contains total stat bonuses for various items
		std::string additional_description = "";
		if (item.getName() == "eldritch_battery") additional_description =
			" (current bonus +" + 
			std::to_string(static_cast<int>(100 * artifacts::ELDRITCH_BATTERY_REGEN_BOOST * quantity)) + 
			"%)";
		else if (item.getName() == "power_shard") additional_description =
			" (current bonus +" +
			std::to_string(static_cast<int>(100 * artifacts::POWER_SHARD_DMG_BOOST * quantity)) +
			"%)";
		else if (item.getName() == "spider_signet") additional_description =
			" (current bonus +" +
			std::to_string(static_cast<int>(100 * artifacts::SPIDER_SIGNET_JUMP_BOOST * quantity)) +
			"%)";
		else if (item.getName() == "watching_eye") additional_description =
			" (current bonus +" +
			std::to_string(static_cast<int>(quantity)) +
			")";
		else if (item.getName() == "bone_mask") additional_description =
			" (current reduction -" +
			std::to_string(static_cast<int>(100 * (1. - std::pow(1. - artifacts::BONE_MASK_PHYS_DMG_REDUCTION, quantity)))) +
			"%)";
		else if (item.getName() == "magic_negator") additional_description =
			" (current reduction -" +
			std::to_string(static_cast<int>(100 * (1. - std::pow(1. - artifacts::MAGIC_NEGATOR_MAGIC_DMG_REDUCTION, quantity)))) +
			"%)";
		else if (item.getName() == "twin_souls") additional_description =
			" (current reduction -" +
			std::to_string(static_cast<int>(100 * (1. - std::pow(1. - artifacts::TWIN_SOULS_CHAOS_DMG_REDUCTION, quantity)))) +
			"%)";

		this->font->color_set(TEXT_COLOR);

		// Icon
		item.drawAt(cursor + OFFSET_ICON);
		// Label
		this->font->draw_line(cursor + OFFSET_LABEL, label);
		// Quantity
		this->font->draw_line(cursor + OFFSET_FOUND, found);
		// Description
		this->font->draw_line(cursor + OFFSET_DESCRIPTION, description + additional_description);

		cursor.y += ICON_SIZE + CELL_GAP;
	}
}

void GUI_Inventory::draw_old() const {
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
	auto &inventory = Game::ACCESS->level->player->inventory;

	Vector2d cursor = GRID_CORNER;

	for (auto& stack : inventory.stacks) {
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
	percentage(1.)
{
	sprite.setTexture(Graphics::ACCESS->getTexture_GUI("player_healthbar.png"));
}

void GUI_PlayerHealthbar::update(Milliseconds elapsedTime) {
	this->percentage = Game::READ->level->player->health->percentage();
}

void GUI_PlayerHealthbar::draw() {
	// Position of healthbar on the screen
	constexpr double HEALTHBAR_LEFT = 2.;
	constexpr double HEALTHBAR_BOTTOM = natural::HEIGHT - 14.;

	// Source rects on the texture
	constexpr auto BORDER_CORNER = Vector2(0, 0);
	constexpr auto BORDER_SIZE = Vector2(16, 82);

	constexpr auto FILL_CORNER = Vector2(17, 1);
	constexpr auto FILL_SIZE = Vector2(14, 80);

	constexpr auto FILL_ALIGMENT = Vector2d(1., 1.); // aligment of fill corner relative to border on the screen

	// Draw fill
	const int fillDisplayedHeight = static_cast<int>(FILL_SIZE.y * this->percentage);

	sprite.setTextureRect(sf::IntRect(
		FILL_CORNER.x,
		FILL_CORNER.y + FILL_SIZE.y - fillDisplayedHeight,
		FILL_SIZE.x,
		fillDisplayedHeight
	));

	sprite.setPosition(sf::Vector2f(
		static_cast<float>(HEALTHBAR_LEFT + FILL_ALIGMENT.x),
		static_cast<float>(HEALTHBAR_BOTTOM - BORDER_SIZE.y + FILL_ALIGMENT.y + FILL_SIZE.y - fillDisplayedHeight)
	));

	// no need to scale

	Graphics::ACCESS->gui->draw_sprite(this->sprite);

	// Draw border
	sprite.setTextureRect(sf::IntRect(
		BORDER_CORNER.x,
		BORDER_CORNER.y,
		BORDER_SIZE.x,
		BORDER_SIZE.y
	));

	sprite.setPosition(
		static_cast<float>(HEALTHBAR_LEFT),
		static_cast<float>(HEALTHBAR_BOTTOM - BORDER_SIZE.y)
	);

	// no need to scale

	Graphics::ACCESS->gui->draw_sprite(this->sprite);
}



// # GUI_PlayerCharges #
namespace GUI_PlayerCharges_consts {
	// Positioning
	constexpr double Y_TOP = natural::HEIGHT - 22 + 2;

	// 3 charges max
	constexpr double X_START_3 = 30;
	constexpr double X_GAP_3 = 6;

	// 4 charges max
	constexpr double X_START_4 = 26;
	constexpr double X_GAP_4 = 4;

	// 5 charges max
	constexpr double X_START_5 = 24;
	constexpr double X_GAP_5 = 2;

	// Source rects
	const sf::IntRect FILL_SOURCE_RECT = {
		0,
		0,
		4,
		4
	};

	const sf::IntRect BORDER_SOURCE_RECT = {
		0,
		4,
		4,
		4
	};

	// Dest rects
	constexpr double FILL_DEST_WIDTH = 8;
	constexpr double FILL_DEST_HEIGHT = 8;

	constexpr double BORDER_DEST_WIDTH = 8;
	constexpr double BORDER_DEST_HEIGHT = 8;
}

GUI_PlayerCharges::GUI_PlayerCharges() {
	using namespace GUI_PlayerCharges_consts;

	this->sprite.setTexture(Graphics::ACCESS->getTexture_GUI("player_charge.png"));
}

void GUI_PlayerCharges::update(Milliseconds elapsedTime) {}

void GUI_PlayerCharges::draw() {
	using namespace GUI_PlayerCharges_consts;

	const uint current = Game::READ->level->player->charges_current;
	const uint max = Game::READ->level->player->charges_max;

	// Spacing of charges (depends on max charges)
	const auto &start =
		max == 3 ? X_START_3 :
		max == 4 ? X_START_4 :
		X_START_5;

	const auto &gap =
		max == 3 ? X_GAP_3 :
		max == 4 ? X_GAP_4 :
		X_GAP_5;

	// Source rects

	// Draw fill
	for (uint i = 0; i < current; ++i) {
		const dstRect fillDestRect = {
			start + i * (BORDER_DEST_WIDTH + gap),
			Y_TOP,
			FILL_DEST_WIDTH,
			FILL_DEST_HEIGHT
		};

		this->sprite.setTextureRect(FILL_SOURCE_RECT);

		this->sprite.setPosition(
			static_cast<float>(start + i * (BORDER_DEST_WIDTH + gap)),
			static_cast<float>(Y_TOP)
		);

		this->sprite.setScale(
			static_cast<float>(FILL_DEST_WIDTH / FILL_SOURCE_RECT.width),
			static_cast<float>(FILL_DEST_HEIGHT / FILL_SOURCE_RECT.height)
		);

		Graphics::ACCESS->gui->draw_sprite(this->sprite);
	}

	// Draw borders
	for (uint i = 0; i < max; ++i) {
		this->sprite.setTextureRect(BORDER_SOURCE_RECT);

		this->sprite.setPosition(
			static_cast<float>(start + i * (BORDER_DEST_WIDTH + gap)),
			static_cast<float>(Y_TOP)
		);

		this->sprite.setScale(
			static_cast<float>(BORDER_DEST_WIDTH / BORDER_SOURCE_RECT.width),
			static_cast<float>(BORDER_DEST_HEIGHT / BORDER_SOURCE_RECT.height)
		);

		Graphics::ACCESS->gui->draw_sprite(this->sprite);
	}
}



// # GUI_PlayerPortrait
GUI_PlayerPortrait::GUI_PlayerPortrait() {
	sprite.setTexture(Graphics::ACCESS->getTexture_GUI("player_portrait.png"));

	// Set size based on texture
	this->size = Vector2d(sprite.getTexture()->getSize().x, sprite.getTexture()->getSize().y);
}

void GUI_PlayerPortrait::update(Milliseconds elapsedTime) {}

void GUI_PlayerPortrait::draw() {
	constexpr double PORTRAIT_LEFT = 25.;
	constexpr double PORTRAIT_BOTTOM = natural::HEIGHT - 14. - 10. + 2;

	sprite.setPosition(
		static_cast<float>(PORTRAIT_LEFT),
		static_cast<float>(PORTRAIT_BOTTOM - this->size.y)
	);

	Graphics::ACCESS->gui->draw_sprite(this->sprite);
}



// # StaticFade #
GUI_Fade::GUI_Fade(const RGBColor &color) :
	color(color)
{
	this->sprite.setTexture(Graphics::ACCESS->getTexture_GUI("fade.png"));
		// a texture of literally white screen
}

void GUI_Fade::update(Milliseconds elapsedTime) {}
void GUI_Fade::draw() {
	this->sprite.setColor(sf::Color(this->color.r, this->color.g, this->color.b, this->color.alpha));

	Graphics::ACCESS->gui->draw_sprite(this->sprite);
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



// # GUI_LevelName #
namespace GUI_LevelName_consts {
	constexpr RGBColor TEXT_COLOR = colors::SH_YELLOW;

	constexpr Vector2d TEXT_CENTER_POSITION{ natural::DIMENSIONS.x * 0.5, natural::DIMENSIONS.y * 0.2 };
	
	constexpr Milliseconds TIME_OPAQUE = sec_to_ms(2.);
	constexpr Milliseconds TIME_FADING = sec_to_ms(3.);
	
	constexpr Milliseconds TIME_TOTAL = TIME_OPAQUE + TIME_FADING;
}

GUI_LevelName::GUI_LevelName(Font* font) :
	font(font),
	in_progress(true),
	time_elapsed(0)
{
	using namespace GUI_LevelName_consts;

	this->text_handle = Graphics::ACCESS->gui->make_line_centered(
		"Entering " + Game::READ->level->getName() + "...",
		TEXT_CENTER_POSITION
	);
	this->text_handle.get().set_overlay(true);
	this->text_handle.get().set_scale(2);
}

GUI_LevelName::~GUI_LevelName() {
	if (this->in_progress) this->text_handle.erase();
		// if (!in_progress+ => text is already erased
}

void GUI_LevelName::update(Milliseconds elapsedTime) {
	if (!this->in_progress) return;  // don't do anything if the text is already gone

	this->time_elapsed += elapsedTime;
}

void GUI_LevelName::draw() {
	using namespace GUI_LevelName_consts;

	if (!this->in_progress) return; // don't do anything if the text is already gone

	if (this->time_elapsed > TIME_TOTAL) {
		this->in_progress = false;
		this->text_handle.erase(); // avoids useless draw() calls
		return;
	}

	const double fade_progress = static_cast<double>((this->time_elapsed - TIME_OPAQUE) / TIME_FADING);

	const RGBColor text_color = (fade_progress < 0)
		// text is still fully opaque
		? TEXT_COLOR 
		// text is partially transparent
		: TEXT_COLOR * (1. - fade_progress) + TEXT_COLOR.transparent() * fade_progress; 

	this->text_handle.get().set_color(text_color);
	///this->font->draw_line_centered(TEXT_CENTER_POSITION, Game::READ->level->getName());
}


// # Gui #
Gui::Gui() :
	fade_override_gui(false)
{
	std::cout << "Creating GUI graphics...\n";

	this->fonts["BLOCKY"] = (std::make_unique<Font>(
		&Graphics::ACCESS->getTexture_GUI("font.png"),
		Vector2(5, 5),
		Vector2d(1., 2.)
		));
}

void Gui::update(Milliseconds elapsedTime) {
	// Handle GUI-realted inputs handling goes here
	auto &game = Game::ACCESS;
	auto &input = game->input;
	
	// NOTE: Inventory and EscMenu GUI's are mutually exclusive
	// Toggling EscMenu
	if (input.key_pressed(Controls::READ->ESC) && game->is_running() && !game->level->player->health->dead() && !this->inventory_menu) {
		game->request_toggleEscMenu();
	}
	// Toggling inventory
	if (input.key_pressed(Controls::READ->INVENTORY) && game->is_running() && !game->level->player->health->dead() && !this->esc_menu) {
		game->request_toggleInventory();
	}
	// Using <Esc> to leave inventory
	else if (this->inventory_menu && input.key_pressed(Controls::READ->ESC)) {
		game->request_toggleInventory();
	}
	// Toggling F3
	if (input.key_pressed(Controls::READ->F3) && game->is_running()) {
		game->request_toggleF3();
	}

	if (this->fade) { this->fade->update(elapsedTime); }

	if (this->FPS_counter) this->FPS_counter->update(elapsedTime);
	if (this->main_menu) this->main_menu->update(elapsedTime);
	if (this->ending_screen) this->ending_screen->update(elapsedTime);
	if (this->esc_menu) this->esc_menu->update(elapsedTime);
	if (Game::READ->is_running() && this->inventory_menu) this->inventory_menu->update(elapsedTime);
	if (Game::READ->is_running() && this->player_healthbar) this->player_healthbar->update(elapsedTime);
	if (Game::READ->is_running() && this->player_charges) this->player_charges->update(elapsedTime);
	if (Game::READ->is_running() && this->player_portrait) this->player_portrait->update(elapsedTime);
	if (Game::READ->is_running() && this->level_name) this->level_name->update(elapsedTime);

	for (auto &text : this->texts) text.update(elapsedTime);
}

void Gui::draw() const {
	if (this->fade && !this->fade_override_gui) this->fade->draw(); // if fade doesn't override GUI

	if (this->FPS_counter && (Game::READ->show_fps_counter || Game::READ->toggle_F3)) this->FPS_counter->draw();
		// FPS counter respects 'show FPS counter' setting, but still renders in F3 mode
	if (this->main_menu) this->main_menu->draw();
	if (this->ending_screen) this->ending_screen->draw();
	if (this->esc_menu) this->esc_menu->draw();
	if (Game::READ->is_running() && this->inventory_menu) this->inventory_menu->draw();
	if (Game::READ->is_running() && this->player_healthbar) this->player_healthbar->draw();
	if (Game::READ->is_running() && this->player_charges) this->player_charges->draw();
	if (Game::READ->is_running() && this->player_portrait) this->player_portrait->draw();
	if (Game::READ->is_running() && this->level_name) this->level_name->draw();

	if (!this->esc_menu && !this->inventory_menu)
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
		this->FPS_counter = std::make_unique<GUI_FPSCounter>(this->fonts.at("BLOCKY").get());
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

// Ending screen
void Gui::EndingScreen_on() {
	if (!this->ending_screen)
		this->ending_screen = std::make_unique<GUI_EndingScreen>(this->fonts.at("BLOCKY").get());
}

void Gui::EndingScreen_off() {
	this->ending_screen.reset();
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
	if (!this->inventory_menu) {
		// Darken the screen when in inventory
		this->Fade_on(EscMenu_consts::COLOR_FADE, false);

		// Disable all player GUI (or it gets in the way)
		///this->AllPlayerGUI_off();

		this->inventory_menu = std::make_unique<GUI_Inventory>(this->fonts.at("BLOCKY").get());
	}
}

void Gui::Inventory_off() {
	// Remove fade
	this->Fade_off();

	// Reenable player GUI
	///this->AllPlayerGUI_on();

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

// Player charges
void Gui::PlayerCharges_on() {
	if (!this->player_charges)
		this->player_charges = std::make_unique<GUI_PlayerCharges>();
}

void Gui::PlayerCharges_off() {
	this->player_charges.reset();
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
	this->PlayerCharges_on();
	this->Portrait_on();
}

void Gui::AllPlayerGUI_off() {
	this->PlayerHealthbar_off();
	this->PlayerCharges_off();
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

// Level name
void Gui::LevelName_on() {
	if (!this->level_name)
		this->level_name = std::make_unique<GUI_LevelName>(this->fonts.at("BLOCKY").get());
}

void Gui::LevelName_off() {
	this->level_name.reset();
}


void Gui::draw_sprite(sf::Sprite &sprite) {
	// Set up GUI view
	sf::View gui_view;
	gui_view.setCenter(
		static_cast<float>(natural::DIMENSIONS.x / 2.), 
		static_cast<float>(natural::DIMENSIONS.y / 2.)
	);
	gui_view.setSize(
		static_cast<float>(natural::DIMENSIONS.x),
		static_cast<float>(natural::DIMENSIONS.y)
	);

	Graphics::ACCESS->window.setView(gui_view);

	// Draw
	Graphics::ACCESS->window_draw_sprite(sprite);
}