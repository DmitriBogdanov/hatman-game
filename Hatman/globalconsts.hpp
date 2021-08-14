#pragma once

#include <string>

#include "vector2.hpp" // 'Vector2d' type



// natural::
// - Natural aka 'logical'/'unscaled' size of various things 
namespace natural {
	constexpr int WIDTH = 640; // natural screen size
	constexpr int HEIGHT = 360;
	constexpr auto DIMENSIONS = Vector2d(WIDTH, HEIGHT);

	constexpr int TILE_SIZE = 32;
	constexpr int ITEM_SIZE = 16;
};



// performance::
// - Consts related to performance
namespace performance {
	constexpr int TILE_FREEZE_RANGE_X = 15; // tiles past that range (from player cell) are not updated
	constexpr int TILE_FREEZE_RANGE_Y = 11;
	constexpr int TILE_DRAW_RANGE_X = 11; // tiles past that range (from player cell) are not drawn
	constexpr int TILE_DRAW_RANGE_Y = 7;

	constexpr int COLLISION_CHECK_DEPH = 1;
		// determines amound of tiles around solids that are checked for collisions
		// collisions are checked for cells in (32 + 2 * COLLISION_CHECK_DEPH)^2 square
		// centered at entity position

	constexpr int ENTITY_FREEZE_RANGE_X = (TILE_FREEZE_RANGE_X + 1) * natural::TILE_SIZE; // entities past that range are not updated
	constexpr int ENTITY_FREEZE_RANGE_Y = (TILE_FREEZE_RANGE_Y + 1) * natural::TILE_SIZE;
	constexpr int ENTITY_DRAW_RANGE_X = 320 + 32; // entities past that range are not drawn
	constexpr int ENTITY_DRAW_RANGE_Y = 192 + 32;
}



// physics::
// - Fundamental physical consts (like gravity)
namespace physics {
	constexpr double GRAVITY_ACCELERATION = 1200.;

	constexpr double PLATFORM_EPSILON = 3.; // how
}



// defaults::
// - Default values for mostly visual settings
namespace defaults {
	constexpr double LEVEL_CHANGE_FADE_DURATION = 500.;
}



// audio::
namespace audio {
	constexpr double MUSIC_VOLUME = 10. / 100.;
	constexpr double FX_VOLUME = 8. / 100.;
}
