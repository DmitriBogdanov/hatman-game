#pragma once

#include <SDL.h>



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

	constexpr RGBColor transparent() const { // returns fully transparent version of a color
		return RGBColor(this->r, this->g, this->b, 0);
	} 
};

namespace colors {
	const RGBColor SH_BLACK(22, 22, 22);
	const RGBColor SH_YELLOW(251, 242, 54);
	const RGBColor SH_BLUE(95, 205, 228);
	const RGBColor SH_GREEN(0, 255, 0);

	/// Old colors, remove later
	const RGBColor WHITE(255, 255, 255);
	const RGBColor BLACK(0, 0, 0);

	const RGBColor IVORY(255, 243, 214);
}