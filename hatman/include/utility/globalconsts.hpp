#pragma once

#include <string>

#include "utility/vector2.hpp" // 'Vector2d' type



// natural::
// - Natural aka 'logical'/'unscaled' size of various things 
namespace natural {
	constexpr int WIDTH = 640; // natural screen size
	constexpr int HEIGHT = 360;
	constexpr auto DIMENSIONS = Vector2d(WIDTH, HEIGHT);

	constexpr int TILE_SIZE = 32;
	constexpr int ITEM_SIZE = 16;

	constexpr double ZOOM = 0.5;
};



// performance::
// - Consts related to performance
namespace performance {
	constexpr double MAX_FRAME_TIME_MS = 10.;
		// physics start to slow down below 1000 / 40 == 25 FPS

	constexpr int TILE_FREEZE_RANGE_X = static_cast<int>(0.5 * natural::WIDTH / natural::TILE_SIZE * natural::ZOOM) + 3;
	constexpr int TILE_FREEZE_RANGE_Y = static_cast<int>(0.5 * natural::HEIGHT / natural::TILE_SIZE * natural::ZOOM) + 3;
		// tiles past that range (from player cell) are not updated
	constexpr int TILE_DRAW_RANGE_X = static_cast<int>(0.5 * natural::WIDTH / natural::TILE_SIZE * natural::ZOOM) + 1;
	constexpr int TILE_DRAW_RANGE_Y = static_cast<int>(0.5 * natural::HEIGHT / natural::TILE_SIZE * natural::ZOOM) + 1;
		// tiles past that range (from player cell) are not drawn

	constexpr int ENTITY_FREEZE_RANGE_X = (TILE_FREEZE_RANGE_X - 1) * natural::TILE_SIZE; // entities past that range are not updated
	constexpr int ENTITY_FREEZE_RANGE_Y = (TILE_FREEZE_RANGE_Y - 1) * natural::TILE_SIZE;
	constexpr int ENTITY_DRAW_RANGE_X = (TILE_DRAW_RANGE_X + 2) * natural::TILE_SIZE; // entities past that range are not drawn
	constexpr int ENTITY_DRAW_RANGE_Y = (TILE_DRAW_RANGE_Y + 2) * natural::TILE_SIZE;
}



// physics::
// - Fundamental physical consts (like gravity)
namespace physics {
	constexpr double GRAVITY_ACCELERATION = 1200.;

	constexpr double PLATFORM_EPSILON = 7.; // how
}



// defaults::
// - Default values for mostly visual settings
namespace defaults {
	constexpr double TRANSITION_FADE_DURATION = 500.;
	constexpr double MUSIC_FADE_DURATION = TRANSITION_FADE_DURATION * 4.;
	constexpr double GAME_ENDING_FADE_DURATION = 8'000.;
}



// audio::
namespace audio {
	constexpr double MUSIC_BASE_VOLUME = 60. / 100.;
	constexpr double FX_BASE_VOLUME = 48. / 100.;
}
