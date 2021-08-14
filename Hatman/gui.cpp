#include "gui.h"

#include "graphics.h" // access to rendering
#include "globalconsts.hpp" // natural consts
#include "item_base.h" // 'Inventory' and 'Item' classes (inventory GUI)
#include "game.h" // access to game state
#include "controls.h" // access to control keys



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



// # GUI_PlayerHealthbar #
GUI_PlayerHealthbar::GUI_PlayerHealthbar() {
	this->texture_border = Graphics::ACCESS->getTexture_GUI("player_healthbar_border.png");
	this->texture_fill = Graphics::ACCESS->getTexture_GUI("player_healthbar_fill.png");
}

void GUI_PlayerHealthbar::update(Milliseconds elapsedTime) {
	this->percentage = Game::READ->level.player->health->percentage();
}
void GUI_PlayerHealthbar::draw() const {
	// Position/size/aligment consts
	const Vector2 fill_size(10, 71); // size of the texture
	const Vector2 border_size(20, 93); // size of the texture

	const Vector2d fill_aligment(5., 11.); // aligment of fill relative to border
	const double fill_bottom_aligment = 82.; // position of the bottom side of hp-bar
	const Vector2d fill_visibleSize(fill_size.x, fill_size.y * this->percentage); // hp size based on hp percentage

	const srcRect sourceRect_fill = {
		0,
		static_cast<int>(fill_size.y * (1. - this->percentage)),
		static_cast<int>(fill_visibleSize.x),
		static_cast<int>(fill_visibleSize.y)
	};

	dRect destRect_fill(this->position + fill_aligment, fill_visibleSize);
	destRect_fill.moveBottomTo(this->position.y + fill_bottom_aligment); // move hpbar in proper place	

	const dRect destRect_border(this->position, border_size);

	const auto destRect_fill_to_dstRect = destRect_fill.to_dstRect();
	const auto destRect_border_to_dstRect = destRect_border.to_dstRect();

	// Draw
	Graphics::ACCESS->gui->textureToGUI(this->texture_fill, &sourceRect_fill, &destRect_fill_to_dstRect);
	Graphics::ACCESS->gui->textureToGUI(this->texture_border, NULL, &destRect_border_to_dstRect);
}



// # GUI_CDbar #
GUI_CDbar::GUI_CDbar() {
	this->texture_border = Graphics::ACCESS->getTexture_GUI("cdbar_border.png");
	this->texture_fill = Graphics::ACCESS->getTexture_GUI("cdbar_fill.png");
}

void GUI_CDbar::update(Milliseconds elapsedTime) {
	/// CHECK PLAYER CD PERCENTAGE
	///this->percentage = Game::READ->level.player->health->percentage(); /// TEMP
	///const Timer &cooldown = Game::READ->level.player->form_change_cooldown;

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



// # GUI_Portrait
GUI_Portrait::GUI_Portrait() {
	this->texture = Graphics::ACCESS->getTexture_GUI("portrait_human.png");

	// Set size
	SDL_QueryTexture(this->texture, NULL, NULL, &this->size.x, &this->size.y);
}

void GUI_Portrait::update(Milliseconds elapsedTime) {}

void GUI_Portrait::draw() const {
	const dstRect destRect = {
		this->position.x - this->size.x / 2.,
		this->position.y - this->size.y,
		static_cast<double>(this->size.x),
		static_cast<double>(this->size.y)
	};

	Graphics::ACCESS->gui->textureToGUI(this->texture, NULL, &destRect);
}



// # FormSelection #
const double FORM_SELECTION_TIMESCALE_FACTOR = 0.2 ;

FormSelection::FormSelection() :
	original_timescale(Game::READ->timescale)
{
	Game::ACCESS->timescale = this->original_timescale * FORM_SELECTION_TIMESCALE_FACTOR;

	//Input &input = Game::ACCESS->input;
	/// WHAT TO DO WITH A FORM CHANGE GUI?
	/*
	if (input.key_held(Controls::READ->FORM_CHANGE_LEFT)) {
		this->selected = Side::LEFT;
	}
	else if (input.key_held(Controls::READ->FORM_CHANGE_RIGHT)) {
		this->selected = Side::RIGHT;
	}
	else if (input.key_held(Controls::READ->FORM_CHANGE_UP)) {
		this->selected = Side::TOP;
	}
	else if (input.key_held(Controls::READ->FORM_CHANGE_DOWN)) {
		this->selected = Side::BOTTOM;
	}
	else {
		this->selected = Side::NONE;
	}
	*/
}

FormSelection::~FormSelection() {
	/// CHANGE PLAYER FORM TO SELECTED
	Game::ACCESS->timescale = this->original_timescale;
}

void FormSelection::update(Milliseconds elapsedTime) {
	//Input &input = Game::ACCESS->input;

	// Last pressed key sets the selected option (1 out of 5)
	/// WHAT TO DO WITH A FORM CHANGE GUI?
	/*
	if (input.key_pressed(Controls::READ->FORM_CHANGE_LEFT)) {
		this->selected = Side::LEFT;
	}
	else if (input.key_pressed(Controls::READ->FORM_CHANGE_RIGHT)) {
		this->selected = Side::RIGHT;
	}
	else if (input.key_pressed(Controls::READ->FORM_CHANGE_UP)) {
		this->selected = Side::TOP;
	}
	else if (input.key_pressed(Controls::READ->FORM_CHANGE_DOWN)) {
		this->selected = Side::BOTTOM;
	}
	else if (!
		(input.key_held(Controls::READ->FORM_CHANGE_LEFT) ||
		input.key_held(Controls::READ->FORM_CHANGE_RIGHT) ||
		input.key_held(Controls::READ->FORM_CHANGE_UP) ||
		input.key_held(Controls::READ->FORM_CHANGE_DOWN))
		) {

		this->selected = Side::NONE;
	}
	*/
}

void FormSelection::draw() const {
	// Textures
	SDL_Texture* hatTexture = Graphics::ACCESS->getTexture_GUI("hat.png");

	// Consts
	const Vector2 hatTextureSize(8, 6);
	const int hatDistanceFromPlayer = 18;

	// Positions
	dRect hatDestRect(Game::READ->level.player->position.toVector2(), hatTextureSize, true);
	double hatRotation = 0.0;

	switch (this->selected) {
	case Side::TOP:
		hatDestRect.moveBy(0, -hatDistanceFromPlayer);
		break;

	case Side::LEFT:
		hatDestRect.moveBy(-hatDistanceFromPlayer, 0);
		hatRotation = -90.0;
		break;

	case Side::BOTTOM:
		hatDestRect.moveBy(0, hatDistanceFromPlayer);
		hatRotation = 180.0;
		break;

	case Side::RIGHT:
		hatDestRect.moveBy(hatDistanceFromPlayer, 0);
		hatRotation = 90.0;
		break;

	case Side::NONE:
		const Vector2 hatAligment(0, -14); // aligns hat to be precisely on characters head
		hatDestRect.moveBy(hatAligment.x, hatAligment.y);
		break;
	}

	const auto hatDestRect_to_dstRect = hatDestRect.to_dstRect();

	Graphics::ACCESS->camera->textureToCameraEx(hatTexture, NULL, &hatDestRect_to_dstRect, hatRotation, SDL_FLIP_NONE);
		// hat is drawn to CAMERA, not GUI!
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


// # Gui::InventoryGUI #
Gui::InventoryGUI::InventoryGUI() :
	currentTab(Tab::ITEMS),
	visible(false)
{
	this->tab_items_texture = Graphics::ACCESS->getTexture_GUI("tab_inventory.png");
	this->position = Vector2d(
		natural::WIDTH / 2. - this->tab_size.x / 2.,
		natural::HEIGHT / 2. - this->tab_size.y / 2.);
}

void Gui::InventoryGUI::update(Milliseconds elapsedTime) {}

void Gui::InventoryGUI::draw() const {
	if (this->visible) {
		// draw currently selected tab
		this->draw_tab_current();
	}
}
 
void Gui::InventoryGUI::show() {
	this->visible = true;
}

void Gui::InventoryGUI::hide() {
	this->visible = false;
}

bool Gui::InventoryGUI::toggle() {
	if (this->visible) {
		this->hide();
	}
	else {
		this->show();
	}

	return this->visible;
}

Gui::InventoryGUI::Tab Gui::InventoryGUI::next_tab() {
	/// IMPLEMENT -> SWITCH TO THE NEXT TAB
	return this->currentTab;
}

void Gui::InventoryGUI::draw_tab_current() const {
	// draw currently selected tab
	switch (this->currentTab) {
	case Tab::ITEMS:
		this->draw_tab_items();
		break;

		//other tabs go there
	}
}

void Gui::InventoryGUI::draw_tab_items() const {
	// setup consts
	const Vector2d grid_start = this->position + Vector2(21, 21);
	const Vector2d grid_size(252., 150.);

	const Vector2d cell_size(21., 25.);
	const Vector2d cell_icon(2., 1.);
	const Vector2d cell_stack(
		cell_icon.x + natural::ITEM_SIZE / 2.,
		cell_icon.y + natural::ITEM_SIZE + 1.
	); // CENTER, NOT CORNER

	const Vector2d page_number_1 = this->position + Vector2d(136., 179.);
	const Vector2d page_number_2 = this->position + Vector2d(156., 179.);

	const Inventory &inventory = Game::ACCESS->level.player->inventory;

	Font* const font = Graphics::ACCESS->gui->fonts.at("BLOCKY").get();
	font->color_set(colors::IVORY);

	const Vector2d font_monospace = font->get_font_monospace(); // to avoid excessive calls during iteration
	const Vector2d font_gap = font->get_font_gap(); // to avoid excessive calls during iteration

	// Draw the tab itself
	dstRect destRect = { this->position.x, this->position.y, this->tab_size.x, this->tab_size.y };
	Graphics::ACCESS->gui->textureToGUI(this->tab_items_texture, NULL, &destRect);

	// Draw grid of items (from player inventory) inside the tab
	Vector2d cursor = grid_start;

	for (const auto &stack : inventory.stacks) {
		// move cursor to the next line if out of bounds
		if (cursor.x >= grid_start.x + grid_size.x) {
			cursor.x = grid_start.x;
			cursor.y += cell_size.y;
		}

		// draw item icon at cursor
		stack.item().drawAt(cursor + cell_icon);

		const std::string quantityString = std::to_string(stack.quantity());

		font->draw_line(
			cursor + cell_stack - Vector2d((quantityString.length() * font_monospace.x - font_gap.x) / 2., 0.),
			quantityString
		);

		cursor.x += cell_size.x;
	}

	// Draw page number
	const std::string pageNum1 = "1"; /// TEMP, WILL DISPLAY CURRENT PAGE NUMBER LATER
	const std::string pageNum2 = "4"; /// TEMP, WILL DISPLAY LAST PAGE NUMBER LATER

	font->draw_line(
		page_number_1 - Vector2d((pageNum1.length() * font_monospace.x - font_gap.x) / 2., 0.),
		pageNum1
	);
	font->draw_line(
		page_number_2 - Vector2d((pageNum2.length() * font_monospace.x - font_gap.x) / 2., 0.),
		pageNum2
	);
}


// # Gui #
Gui::Gui() :
	FPS_counter(nullptr)
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
	if (this->fade) { this->fade->update(elapsedTime); }

	this->inventoryGUI.update(elapsedTime); // non-optional

	if (this->FPS_counter) { this->FPS_counter->update(elapsedTime); }
	if (this->player_healthbar) { this->player_healthbar->update(elapsedTime); }
	if (this->cdbar) { this->cdbar->update(elapsedTime); }
	if (this->portrait) { this->portrait->update(elapsedTime); }
	if (this->form_selection) { this->form_selection->update(elapsedTime); }

	for (auto &text : this->texts) { text.update(elapsedTime); }
}

void Gui::draw() const {
	if (this->fade) { this->fade->draw(); }

	this->inventoryGUI.draw(); // non-optional

	if (this->FPS_counter) { this->FPS_counter->draw(); }
	if (this->player_healthbar) { this->player_healthbar->draw(); }
	if (this->cdbar) { this->cdbar->draw(); }
	if (this->portrait) { this->portrait->draw(); }
	if (this->form_selection) { this->form_selection->draw(); }

	for (const auto &text : this->texts) { text.draw(); } // text is drawn on top of everything else
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

// Healthbar
void Gui::PlayerHealthbar_on() {
	if (!this->player_healthbar) {
		this->player_healthbar = std::make_unique<GUI_PlayerHealthbar>();
	}
}
void Gui::PlayerHealthbar_off() {
	this->player_healthbar.reset();
}

// CDbar
void Gui::CDbar_on() {
	if (!this->cdbar) {
		this->cdbar = std::make_unique<GUI_CDbar>();
	}
}
void Gui::CDbar_off() {
	this->cdbar.reset();
}

// Portrait
void Gui::Portrait_on() {
	if (!this->portrait) {
		this->portrait = std::make_unique<GUI_Portrait>();
	}
}
void Gui::Portrait_off() {
	this->portrait.reset();
}

// FormSelection
void Gui::FormSelection_on() {
	if (!this->form_selection) {
		this->form_selection = std::make_unique<FormSelection>();
	}
}
void Gui::FormSelection_off() {
	this->form_selection.reset();
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
void Gui::Fade_on(const RGBColor &color) {
	// New fade replaces existing fade
	if (this->fade) { this->Fade_off(); }

	this->fade = std::make_unique<GUI_Fade>(color);
}
void Gui::Fade_on(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration) {
	if (this->fade) { this->Fade_off(); }

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