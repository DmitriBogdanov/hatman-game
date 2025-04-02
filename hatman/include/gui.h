#pragma once

#include <SFML/Graphics.hpp>

#include <memory> // 'unique_ptr' type
#include <unordered_map> // related type
#include <set> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "collection.hpp" // 'Collection' class
#include "player.h" // 'Forms' enum
#include "color.hpp" // 'RGBColor' type
#include "sound.h" // 'Sound' type for GUI interactions



// # Font #
// - Represents a monospace font, used by 'Text' objects and GUI
// - Can be used to draw single lines and symbols of any color, without creating the 'Text' object
class Font {
public:
	Font() = delete;
	Font(sf::Texture* texture, const Vector2 &size, const Vector2d &gap);

	Vector2d draw_symbol(const Vector2d &position, char symbol, bool overlay = true);
		// returns position of character end (top-right corner)
	Vector2d draw_line(const Vector2d &position, const std::string &line, bool overlay = true);
		// returns position of line end (top-right corner)

	void draw_line_centered(const Vector2d &position, const std::string &line, bool overlay = true);
		// QoL proxy for centered 'draw_line'

	// Color
	void color_set(const RGBColor &color);
	void color_reset();
		// Color modification follows this formula:
		// COLOR.r = COLOR.r * (MODIFIER.r / 255);
		// COLOR.g = COLOR.g * (MODIFIER.g / 255);
		// COLOR.b = COLOR.b * (MODIFIER.b / 255);
		// COLOR.alpha = COLOR.alpha * (MODIFIER.alpha / 255);
		// For that reason (255, 255, 255, 255) white is a 'neutral' color
	void scale_set(double scale);
	void scale_reset();
	
	// Getters
	Vector2 get_font_size() const;
	Vector2d get_font_gap() const;
	Vector2d get_font_monospace() const; // monospace == font_size + font_gap

private:
	sf::Sprite sprite;

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

	///Text(const Text &other) = delete;
	///Text& operator=(const Text &other) = delete;
		/// For some reason explicitly deleting constructor causes issues in "xmemory()" despite
		/// copy never being used in any context. Track the cause of the issue later on.
		// iterators need to be handled in a non-trivial way during copy
		// this can be implemented but better be avoided copy altogether

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
	void set_scale(double scale);

	RGBColor color; // defaults as white (aka no color modifier)

	bool overlay = true; // if false, text is rendered to camera

	bool centered = false;
private:
	std::string content; // necessary to have elements in correct order
	std::set<std::string::const_iterator> line_breaks; // unordered_set can't properly hash iterators

	dRect bounds; // rectangle that contains text

	double scale;

	Font* font;

	Milliseconds delay = 0.; // delay between displaying of each symbol (leave at 0 for instant display)
	Milliseconds time_elapsed = 0.; // records elapsed time since last advance of 'display_end'

	std::string::const_iterator finish; // holds iterator to the last displayable symbol

	void draw_line(const Vector2d &position, const std::string &line) const;

	void setup_line_breaks();
};



//// # GUI_CornerText #
//// - Slowly fading messages that appear in the corner of hte screen
//class GUI_CornerText {
//public:
//	GUI_CornerText() = delete;
//	GUI_CornerText(const std::string& text, const dRect& bounds, Font* font, Milliseconds delay)
//};



// # GUI_FPSCounter #
// - Counts FPS and displays it
class GUI_FPSCounter {
public:
	GUI_FPSCounter() = delete;

	GUI_FPSCounter(Font* font);

	void update(Milliseconds elapsedTime);
	void draw() const;

private:
	Font* font;

	Milliseconds time_elapsed; // DOES NOT ACCOUNT FOR TIMESCALE ( unique property)

	int frames_elapsed;
	int currentFPS;
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

	Sound hover_sound;
	Sound click_sound;

	RGBColor color;
	RGBColor color_hovered;
	RGBColor color_pressed;
};



// # GUI_EscMenu # 
class GUI_EscMenu {
public:
	enum class Tab {
		MAIN,
		CONTROLS
	};

public:
	GUI_EscMenu() = delete;

	GUI_EscMenu(Font* font);

	void update(Milliseconds elapsedTime); // selects different update() base on current tab
	void update_tab_main(Milliseconds elapsedTime);
	void update_tab_controls(Milliseconds elapsedTime);

	void draw() const; // selects different draw() base on current tab
	void draw_tab_main() const;
	void draw_tab_controls() const;

private:
	Font* font;

	Tab current_tab;

	// Main tab
	std::unique_ptr<GUI_Button> button_resume;
	std::unique_ptr<GUI_Button> button_controls;
	std::unique_ptr<GUI_Button> button_start_from_checkpoint;
	std::unique_ptr<GUI_Button> button_return_to_main_menu;
	std::unique_ptr<GUI_Button> button_exit_to_desktop;

	// Controls tab
	std::unique_ptr<GUI_Button> button_back;
};



// # GUI_MainMenu #
class GUI_MainMenu {
public:
	enum class Tab {
		MAIN,
		SETTINGS
	};

public:
	GUI_MainMenu() = delete;

	GUI_MainMenu(Font* font);

	void update(Milliseconds elapsedTime);  // selects different update() base on current tab
	void update_tab_main(Milliseconds elapsedTime);
	void update_tab_settings(Milliseconds elapsedTime);

	void draw(); // selects different draw() base on current tab
	void draw_tab_main();
	void draw_tab_settings();

private:
	Font* font;

	Tab current_tab;

	// Main tab
	std::unique_ptr<GUI_Button> button_continue;
	std::unique_ptr<GUI_Button> button_new_game;
	std::unique_ptr<GUI_Button> button_settings;
	std::unique_ptr<GUI_Button> button_exit;

	// Settings tab
	bool config_was_changed;

	// Resolution
	std::unique_ptr<GUI_Button> resolution_decrease;
	std::unique_ptr<GUI_Button> resolution_increase;
	std::vector<int> resolution_options_x;
	std::vector<int> resolution_options_y;
		// Can't be stored as an outside const due to needing to add custom resolutions to the list when present
	int resolution_current_option;

	// Screen mode
	std::unique_ptr<GUI_Button> screenmode_decrease;
	std::unique_ptr<GUI_Button> screenmode_increase;
	int screenmode_current_option;

	// Music
	std::unique_ptr<GUI_Button> music_decrease;
	std::unique_ptr<GUI_Button> music_increase;
	int music_current_option;

	// Sound
	std::unique_ptr<GUI_Button> sound_decrease;
	std::unique_ptr<GUI_Button> sound_increase;
	int sound_current_option;

	// FPS counter
	std::unique_ptr<GUI_Button> fps_decrease;
	std::unique_ptr<GUI_Button> fps_increase;
	int fps_current_option;

	std::string parsed_save_filepath; // save filepath is propagated the same as was parsed

	// Apply and cancel
	std::unique_ptr<GUI_Button> button_cancel;
	std::unique_ptr<GUI_Button> button_apply;

	// Background
	sf::Sprite sprite; // 128x72 background for main menu
};



// # GUI_EndingScreen #
class GUI_EndingScreen {
public:
	GUI_EndingScreen() = delete;

	GUI_EndingScreen(Font* font);

	void update(Milliseconds elapsedTime);  // selects different update() base on current tab

	void draw(); // selects different draw() base on current tab

private:
	Font* font;
	
	std::unique_ptr<GUI_Button> button_continue_playthrough;
	std::unique_ptr<GUI_Button> button_finish_playthrough;
};



// # GUI_Inventory #
class GUI_Inventory {
public:
	GUI_Inventory() = delete;

	GUI_Inventory(Font* font);

	void update(Milliseconds elapsedTime); // empty
	void draw();

	void draw_old() const;

private:
	Font* font;
};



// # GUI_PlayerHealthbar #
class GUI_PlayerHealthbar {
public:
	GUI_PlayerHealthbar();

	void update(Milliseconds elapsedTime);
	void draw();

private:
	sf::Sprite sprite;

	double percentage;
};



// GUI_PlayerCharges {
class GUI_PlayerCharges {
public:
	GUI_PlayerCharges();

	void update(Milliseconds elapsedTime); // empty
	void draw();

private:
	sf::Sprite sprite;
};



// # GUI_FormPortrait #
class GUI_PlayerPortrait {
public:
	GUI_PlayerPortrait();

	void update(Milliseconds elapsedTime); // empty
	void draw();

private:
	Vector2d size; // equal to the texture size

	sf::Sprite sprite;
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
	void draw();

protected:
	sf::Sprite sprite;

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

// # LevelName #
// - Level name that appears on the screen after entering a new level
// - Fades away after a few seconds
class GUI_LevelName {
public:
	GUI_LevelName() = delete;

	GUI_LevelName(Font *font);

	~GUI_LevelName();

	void update(Milliseconds elapsedTime);
	void draw();

private:
	Font* font;

	bool in_progress; // false <=> text is erased => skip all logic

	Collection<Text>::handle text_handle;

	Milliseconds time_elapsed;
};



// # Gui #
// - Used for rendering all overlays
// - Serves as an interface for creation and managing of all GUI elements and modules
class Gui {
public:
	Gui();

	~Gui() = default;

	void update(Milliseconds elapsedTime);
	void draw() const;

	// Text
	std::unordered_map<std::string, std::unique_ptr<Font>> fonts;
	Collection<Text> texts;

	// pass (overlay == false) to draw as an object rather than as overlay
	Collection<Text>::handle make_text(const std::string &text, const dRect &field); // makes text
	Collection<Text>::handle make_line(const std::string &line, const Vector2d &position); // makes single-line text
	Collection<Text>::handle make_line_centered(const std::string &line, const Vector2d &position);

	// FPSCounter
	void FPSCounter_on();
	void FPSCounter_off();

	// Main menu
	void MainMenu_on();
	void MainMenu_off();

	// Ending screen
	void EndingScreen_on();
	void EndingScreen_off();

	// Esc menu
	void EscMenu_on();
	void EscMenu_off();
	void EscMenu_toggle();

	// Inventory
	void Inventory_on();
	void Inventory_off();
	void Inventory_toggle();

	// Player healthbar
	void PlayerHealthbar_on();
	void PlayerHealthbar_off();

	// Player charges
	void PlayerCharges_on();
	void PlayerCharges_off();

	// Player portrait
	void Portrait_on();
	void Portrait_off();

	void AllPlayerGUI_on(); // includes PlayerHealthbar + CDbar + Portrait
	void AllPlayerGUI_off();

	// Fade
	void Fade_on(const RGBColor &color, bool overrideGUI = true); // makes a static fade effect
	void Fade_on(const RGBColor &colorStart, const RGBColor &colorEnd, Milliseconds duration, bool overrideGUI = true); // makes a smooth fade effect
	void Fade_off();

	// Level name
	void LevelName_on();
	void LevelName_off();

	// General
	void draw_sprite(sf::Sprite &sprite);

private:
	// GUI elements that do NOT need to remember internal state while changing visibility
	std::unique_ptr<GUI_FPSCounter> FPS_counter;
	std::unique_ptr<GUI_MainMenu> main_menu;
	std::unique_ptr<GUI_EndingScreen> ending_screen;
	std::unique_ptr<GUI_EscMenu> esc_menu;
	std::unique_ptr<GUI_Inventory> inventory_menu;
	std::unique_ptr<GUI_PlayerHealthbar> player_healthbar;
	std::unique_ptr<GUI_PlayerCharges> player_charges;
	std::unique_ptr<GUI_PlayerPortrait> player_portrait;
	std::unique_ptr<GUI_Fade> fade;
	std::unique_ptr<GUI_LevelName> level_name;

	bool fade_override_gui; // if false other GUI elements have higher rendering priority than fade
};