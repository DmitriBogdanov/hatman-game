#pragma once

#include <SDL.h> // 'SDL_Texture' type

#include "geometry_utils.h" // geometry types



// # Camera #
// - Handles all calculation of relative positions and etc
// - Contains a backbuffer for rendering
// - Supports tilt (up to ~25 degrees)
// - Supports zoom (from 0 to 2, where < 1 is a zoom-in, > 1 is a zoom-out)
class Camera {
public:
	Camera(const Vector2d &position = Vector2d());

	~Camera(); // frees 'backbuffer' texture

	// FOV
	Vector2d getFOV_Corner() const;
	Vector2d getFOV_Size() const; // returns size of a current Field Of View
	dRect getFOV_Rect() const; // returns rectangle with current Field Of View
	
	// Position conversions
	Vector2d get_ScreenPos_from_LevelPos(const Vector2d &levelPos) const;
	Vector2d get_LevelPos_from_ScreenPos(const Vector2d &screenPos) const;

	void textureToCamera(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect);
		// copies sourceRect from given texture to destinationRect on renderer
	void textureToCameraEx(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect, double angle, SDL_RendererFlip flip);
		// same as above but allows rotation and flips

	void cameraToRenderer();
	void cameraClear();

	Vector2d position;
	double zoom; // max zoom-out is 2
	double angle;

private:
	Vector2d standard_FOV; // standart FOV in natural scale
	Vector2 scaled_standard_FOV; // standart FOV scaled

	SDL_Texture* backbuffer; // requires destruction!
		// a bit bigger than view field to account for small rotation/transitions during screen shake
	Vector2 backbuffer_size; // backbuffer size (not including margin)
	int MARGIN = 200; /// perhaps it should be scaled to fit rendering size
};