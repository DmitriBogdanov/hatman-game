#include "systems/controls.h"



// # Controls #
const Controls* Controls::READ;
Controls* Controls::ACCESS;

Controls::Controls() {
	this->READ = this;
	this->ACCESS = this; 

	// Gui controls
	this->LMB = sf::Mouse::Button::Left;
	this->ESC = sf::Keyboard::Key::Escape;
	this->F3 = sf::Keyboard::Key::F3;

	// Game controls
	this->LEFT = sf::Keyboard::Key::A;
	this->RIGHT = sf::Keyboard::Key::D;
	this->UP = sf::Keyboard::Key::W;
	this->DOWN = sf::Keyboard::Key::S;

	this->CHAIN = sf::Mouse::Button::Left;
	this->SHIFT = sf::Keyboard::Key::LShift;
	this->SKILL = sf::Keyboard::Key::C;

	this->ZOOMOUT = sf::Keyboard::Key::R;

	this->JUMP = sf::Keyboard::Key::Space;
	this->USE = sf::Keyboard::Key::E;

	this->INVENTORY = sf::Keyboard::Key::I;
}

std::string toString(sf::Mouse::Button button) {
	using Button = sf::Mouse::Button;

	switch (button) {
	// Main buttons
	case Button::Left: return "LMB";
	case Button::Middle: return "Middle Button"; 
	case Button::Right: return "RMB";
	case Button::XButton1: return "Extra Button 1";
	case Button::XButton2: return "Extra Button 2";
	// Other buttons
	default: return "Undefined";
	}
}

std::string toString(sf::Keyboard::Key key) {
	using Key = sf::Keyboard::Key;

	switch (key) {
	// Numbers
	case Key::Num1: return "1";
	case Key::Num2: return "2";
	case Key::Num3: return "3";
	case Key::Num4: return "4";
	case Key::Num5: return "5";
	case Key::Num6: return "6";
	case Key::Num7: return "7";
	case Key::Num8: return "8";
	case Key::Num9: return "9";
	case Key::Num0: return "0";
	case Key::Dash: return "Dash";
	case Key::Equal: return "Equals";
	// Row 1
	case Key::Q: return "Q";
	case Key::W: return "W";
	case Key::E: return "E";
	case Key::R: return "R";
	case Key::T: return "T";
	case Key::Y: return "Y";
	case Key::U: return "U";
	case Key::I: return "I";
	case Key::O: return "O";
	case Key::P: return "P";
	case Key::LBracket: return "LBracket";
	case Key::RBracket: return "RBracket";
	// Row 2
	case Key::A: return "A";
	case Key::S: return "S";
	case Key::D: return "D";
	case Key::F: return "F";
	case Key::G: return "G";
	case Key::H: return "H";
	case Key::J: return "J";
	case Key::K: return "K";
	case Key::L: return "L";
	case Key::Semicolon: return "Semicolon";
	case Key::Quote: return "Quote";
	// Row 3
	case Key::Z: return "Z";
	case Key::X: return "X";
	case Key::C: return "C";
	case Key::V: return "V";
	case Key::B: return "B";
	case Key::N: return "N";
	case Key::M: return "M";
	case Key::Comma: return "Comma";
	case Key::Period: return "Period";
	case Key::Slash: return "Slash";
	// Additional keys
	case Key::Space: return "Space";
	case Key::Tab: return "Tab";
	case Key::LShift: return "LShift";
	case Key::RShift: return "RShift";
	case Key::LControl: return "LCtrl";
	case Key::RControl: return "RCtrl";
	case Key::LAlt: return "LAlt";
	case Key::RAlt: return "RAlt";
	// Other keys
	default: return "Undefined";
	}
}