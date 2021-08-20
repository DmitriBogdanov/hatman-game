#pragma once

#include <SDL.h> // 'SDL_Texture' type
#include <memory> // 'unique_ptr' type
#include <unordered_map> // related type
#include <set> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "collection.hpp" // 'Collection' class
#include "player.h" // 'Forms' enum
#include "color.hpp" // 'RGBColor' type



// # Font #
// - Represents a monospace font, used by 'Text' objects and GUI
// - Can be used to draw single lines and symbols of any color, without creating the 'Text' object
class Font {
public:
	Font() = delete;
	Font(SDL_Texture* texture, const Vector2 &size, const Vector2d &gap);

	Vector2d draw_symbol(const Vector2d &position, char symbol, bool overlay = true) const;
		// returns position of character end (top-right corner)
	Vector2d draw_line(const Vector2d &position, const std::string &line, bool overlay = true) const;
		// returns position of line end (top-right corner)

	// Color
	void color_set(const RGBColor &color);
	void color_reset();
		// Color modification follows this formula:
		// COLOR.r = COLOR.r * (MODIFIER.r / 255);
		// COLOR.g = COLOR.g * (MODIFIER.g / 255);
		// COLOR.b = COLOR.b * (MODIFIER.b / 255);
		// COLOR.alpha = COLOR.alpha * (MODIFIER.alpha / 255);
		// For that reason (255, 255, 255, 255) white is a 'neutral' color
	
	// Getters
	Vector2 get_font_size() const;
	Vector2d get_font_gap() const;
	Vector2d get_font_monospace() const; // monospace == font_size + font_gap

private:
	SDL_Texture* font_texture;

	Vector2 font_size; // size of 1 symbol on source texture
	Vector2d font_gap;
};



// # Text #
// - Text as an independent object
// - Used to enable smooth (aka 'animated') text display
// - Supports any monospace fonts, colors and animation delays
class Text {
public:
	Text() = delete;
	Text(const std::string &text, const dRect &bounds, Font* font);

	void update(Milliseconds elapsedTime);
	void draw() const;

	// Getters
	bool is_finished() const; // true if text display is finished / is instant
	const Font& get_font() const;

	// Setters
	void set_properties(const RGBColor &color, bool overlay, bool centered, Milliseconds delay); // for all properites at the same time

	void set_color(const RGBColor &color); // here purely for interface convenience
	void set_color(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255);
	void set_overlay(bool value); // here purely for interface convenience
	void set_centered(bool value); // here purely for interface convenience
	void set_delay(Milliseconds delay);

	RGBColor color = RGBColor(); // defaults as white (aka no color modifier)

	bool overlay = true; // if false, text is rendered to camera

	bool centered = false;
private:
	std::string content; // necessary to have elements in correct order
	std::set<std::string::const_iterator> line_breaks; // unordered_set can't properly hash iterators

	dRect bounds; // rectangle that contains text

	Font* font;

	Milliseconds delay = 0.; // delay between dispaying of each symbol (leave at 0 for instant dispay)
	Milliseconds time_elapsed = 0.; // records elapsed time since last advance of 'display_end'

	std::string::const_iterator finish; // holds iterator to the last displayable symbol

	void draw_line(const Vector2d &position, const std::string &line) const;

	void setup_line_breaks();
};



// # GUI_FPSCounter #
// - Counts FPS and displays it
class GUI_FPSCounter {
public:
	GUI_FPSCounter();

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Vector2d position = Vector2d(2., 352.); // position is de-facto a constant

	Collection<Text>::handle text_handle;

	Milliseconds time_elapsed; // DOES NOT ACCOUNT FOR TIMESCALE ( unique property)

	int frames_elapsed;
	int currentFPS;

	Milliseconds UPDATE_RATE = 1000.; // time between FPS counter updates in ms
};



// # Gui_Button #
class GUI_Button {
public:
	GUI_Button() = delete;

	GUI_Button(const dRect& buttonRect, const std::string& displayedText, Font* font,
		const RGBColor &color, const RGBColor& colorHovered, const RGBColor& colorPressed);

	void update(Milliseconds elapsedTime);
	void draw(); // displays text aligned to the center of the button

	bool was_pressed() const;

	void reset(); // resets .button_was_pressed

private:
	dRect button_rect;

	bool button_hovered_over;
	bool button_is_being_pressed;

	bool button_was_pressed;

	Vector2d text_pos; // we assume button stays static on the screen and precalculate text position
	std::string displayed_text;
	Font* font;

	RGBColor color;
	RGBColor color_hovered;
	RGBColor color_pressed;
};



// # GUI_EscMenu #
class GUI_EscMenu {
public:
	GUI_EscMenu() = delete;

	GUI_EscMenu(Font* font);

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Font* font;

	std::unique_ptr<GUI_Button> button_resume;
	std::unique_ptr<GUI_Button> button_controls;
	std::unique_ptr<GUI_Button> button_start_from_checkpoint;
	std::unique_ptr<GUI_Button> button_return_to_main_menu;
	std::unique_ptr<GUI_Button> button_exit_to_desktop;
};



// # GUI_MainMenu #
class GUI_MainMenu {
public:
	GUI_MainMenu() = delete;

	GUI_MainMenu(Font* font);

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Font* font;

	std::unique_ptr<GUI_Button> button_continue;
	std::unique_ptr<GUI_Button> button_new_game;
	std::unique_ptr<GUI_Button> button_settings;
	std::unique_ptr<GUI_Button> button_exit;

	SDL_Texture* texture; // 640x360 background for main menu
};



// # GUI_PlayerHealthbar #
class GUI_PlayerHealthbar {
public:
	GUI_PlayerHealthbar();

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Vector2d position = Vector2d(2., 255.); // position is de-facto a constant

	SDL_Texture* texture_border;
	SDL_Texture* texture_fill;

	double percentage = 1.;
};


// # GUI_CDbar #
class GUI_CDbar {
public:
	GUI_CDbar();

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Vector2d position = Vector2d(25., 340.); // position is de-facto a constant

	SDL_Texture* texture_border;
	SDL_Texture* texture_fill;

	double percentage = 1.;
};



// # GUI_FormPortrait #
class GUI_Portrait {
public:
	GUI_Portrait();

	void update(Milliseconds elapsedTime); // empty
	void draw() const;

private:
	Vector2d position = Vector2d(61., 336.); // X is for CENTER, Y if for BOTTOM of the image!
	Vector2 size; // equal to the texture size

	SDL_Texture* texture;
};



// # GUI_Fade #
// - Used to apply 'fading' effect to the whole screen
// - Allows any color, but beware of high 'alpha' value
class GUI_Fade {
public:
	GUI_Fade() = delete;
	GUI_Fade(const RGBColor &color);

	virtual ~GUI_Fade() = default;

	virtual void update(Milliseconds elapsedTime); // does nothing
	void draw() const;

protected:
	SDL_Texture* texture;

	RGBColor color;
};



// # SmoothFade #
// - Unlike its simplier parent, smoothly goes from one color to another in a given time
class GUI_SmoothFade : public GUI_Fade {
public:
	GUI_SmoothFade() = delete;
	GUI_SmoothFade(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration);

	~GUI_SmoothFade() = default;

	void update(Milliseconds elapsedTime); // used for smooth color change

private:
	Milliseconds duration;
	Milliseconds time_elapsed;
	
	RGBColor color_start;
	RGBColor color_end;
};



// # Gui #
// - Used for rendering all overlays
// - Serves as an interface for creation and managing of all GUI elements and modules
class Gui {
public:
	Gui();

	~Gui(); // frees 'backbuffer' texture

	void update(Milliseconds elapsedTime);
	void draw() const;

	// Text
	// pass (overlay == false) to draw as an object rather than as overlay
	Collection<Text>::handle make_text(const std::string &text, const dRect &field); // makes text
	Collection<Text>::handle make_line(const std::string &line, const Vector2d &position); // makes single-line text
	Collection<Text>::handle make_line_centered(const std::string &line, const Vector2d &position);

	std::unordered_map<std::string, std::unique_ptr<Font>> fonts;
	Collection<Text> texts;

	/// Entity healthbars
	///void drawHealthbar(const Vector2d &bottomMiddlePosition, double percentage);

	// # Gui::InventoryGUI #
	class InventoryGUI {
	public:
		InventoryGUI(); // sets up textures

		void update(Milliseconds elapsedTime);
		void draw() const;

		void show();
		void hide();
		bool toggle(); // switches visibility to oppposite, returns visibility bool

		enum class Tab {
			ITEMS
		};

		Tab next_tab(); // swithces current tab to next, returns new current tab


		Tab currentTab;

		Vector2d position; // position of top-left corner

	private:
		bool visible;

		void draw_tab_current() const;
		Vector2d tab_size = Vector2d(293., 192.); // size of the inventory tab on the screen

		void draw_tab_items() const;
		SDL_Texture* tab_items_texture;


		//void draw_tab_quests() const;
		//void draw_tab_stats() const;
		//void draw_tab_cards() const;
		//void draw_tab_journal() const;
	};

	// FPSCounter
	void FPSCounter_on();
	void FPSCounter_off();

	// Main menu
	void MainMenu_on();
	void MainMenu_off();

	// Esc menu
	void EscMenu_on();
	void EscMenu_off();
	void EscMenu_toggle();

	// Character healthbar
	void PlayerHealthbar_on();
	void PlayerHealthbar_off();

	// CDbar
	void CDbar_on();
	void CDbar_off();

	// Portrait
	void Portrait_on();
	void Portrait_off();

	void AllPlayerGUI_on(); // includes PlayerHealthbar + CDbar + Portrait
	void AllPlayerGUI_off();

	// Fade
	void Fade_on(const RGBColor &color, bool overrideGUI = true); // makes a static fade effect
	void Fade_on(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration, bool overrideGUI = true); // makes a smooth fade effect
	void Fade_off();


	// General
	void textureToGUI(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect);
	void textureToGUIEx(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect, double angle, SDL_RendererFlip flip);
		// same as above but allows rotation and flips

	void GUIToRenderer();
	void GUIClear();

	// GUI elements that need to remember internal state while changing visibility
	InventoryGUI inventoryGUI;

private:
	SDL_Texture* backbuffer; // requires destruction!

	// GUI elements that do NOT need to remember internal state while changing visibility
	std::unique_ptr<GUI_FPSCounter> FPS_counter;
	std::unique_ptr<GUI_MainMenu> main_menu;
	std::unique_ptr<GUI_EscMenu> esc_menu;
	std::unique_ptr<GUI_PlayerHealthbar> player_healthbar;
	std::unique_ptr<GUI_CDbar> cdbar;
	std::unique_ptr<GUI_Portrait> portrait;
	std::unique_ptr<GUI_Fade> fade;
	bool fade_override_gui; // if false other GUI elements have higher rendering priority than fade
};