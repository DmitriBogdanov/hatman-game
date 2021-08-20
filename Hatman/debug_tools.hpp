#pragma once

#include "graphics.h"



// # DEBUG_SINGLETON #
// - Used for faster testing
// - Can display tracked values on the screen during gameplay
// - Handles formatting by itself
// - NOT to be included in release build
class DEBUG_SINGLETON {
public:
	DEBUG_SINGLETON(const DEBUG_SINGLETON& other) = delete;
	void operator = (const DEBUG_SINGLETON& other) = delete;

	static DEBUG_SINGLETON& get() {
		static DEBUG_SINGLETON instance;

		return instance;
	}

	inline void begin_new_frame() {
		this->value_count = 0;
	}

	template<class Value>
	void draw_value(const std::string& message, const Value& value) {
		constexpr auto gap = Vector2d(200., 2.);
		constexpr double gapX = 120.;
		constexpr double gapY = 8.;

		Font* font = Graphics::ACCESS->gui->fonts.at("BLOCKY").get();
		font->color_set(colors::SH_GREEN);

		font->draw_line(gap + Vector2d(gapX * 0, gapY * this->value_count), message + ":");
		font->draw_line(gap + Vector2d(gapX * 1, gapY * this->value_count), std::to_string(value));

		++this->value_count;
	}

private:
	DEBUG_SINGLETON() : value_count(0) {}

	int value_count;
};