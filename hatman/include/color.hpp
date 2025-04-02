#pragma once

#include <cstdint>



using Uint8 = std::uint8_t;

// # Color #
// - Represents an RGB color with transparency
struct RGBColor {
	constexpr RGBColor() :
		r(255), g(255), b(255), alpha(255) // default color is non-transparent (255, 255, 255) white
	{} 
	constexpr RGBColor(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255) :
		r(r), g(g), b(b), alpha(alpha)
	{}

	Uint8 r, g, b;
	Uint8 alpha;

	constexpr RGBColor operator*(double coef) const {
		return RGBColor(
			static_cast<Uint8>(this->r * coef),
			static_cast<Uint8>(this->g * coef),
			static_cast<Uint8>(this->b * coef),
			static_cast<Uint8>(this->alpha * coef)
		);
	}

	constexpr RGBColor operator+(const RGBColor &other) const {
		return RGBColor(
			this->r + other.r,
			this->g + other.g,
			this->b + other.b,
			this->alpha + other.alpha
		);
	}

	constexpr RGBColor set_alpha(Uint8 alpha) const { // returns partially transparent version of a color
		return RGBColor(this->r, this->g, this->b, alpha);
	}

	constexpr RGBColor transparent() const { // returns fully transparent version of a color
		return RGBColor(this->r, this->g, this->b, 0);
	} 
};

namespace colors {
	constexpr auto SH_BLACK = RGBColor(22, 22, 22);
	constexpr auto  SH_YELLOW = RGBColor(251, 242, 54);
	constexpr auto  SH_BLUE = RGBColor(95, 205, 228);
	constexpr auto  SH_GREEN = RGBColor(0, 255, 0);

	constexpr auto ESC_MENU_FADE_COLOR = SH_BLACK.set_alpha(100);

	constexpr auto  WHITE = RGBColor(255, 255, 255);
	constexpr auto  FULL_BLACK = RGBColor(0, 0, 0);
}