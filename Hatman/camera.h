#pragma once

#include <SDL.h> // 'SDL_Texture' type
#include <SFML/Graphics.hpp>

#include "geometry_utils.h" // geometry types



// # Camera #
// - Handles all calculation of relative positions and etc
// - Contains a backbuffer for rendering
// - Supports tilt (up to ~25 degrees)
// - Supports zoom (from 0 to 2, where < 1 is a zoom-in, > 1 is a zoom-out)
class Camera {
public:
	Camera(const Vector2d &position = Vector2d());

	~Camera() = default; // frees 'backbuffer' texture

	// FOV
	Vector2d get_FOV_corner() const;
	Vector2d get_FOV_size()   const; // returns size of a current Field Of View
	dRect    get_FOV_rect()   const; // returns rectangle with current Field Of View
	
	// Position conversions
	Vector2d get_ScreenPos_from_LevelPos(const Vector2d &levelPos) const;
	Vector2d get_LevelPos_from_ScreenPos(const Vector2d &screenPos) const;

	void draw_sprite(sf::Sprite &sprite);

	Vector2d position;

	//void textureToCamera(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect);
	//	// copies sourceRect from given texture to destinationRect on renderer
	//void textureToCameraEx(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect, double angle, SDL_RendererFlip flip);
	//	// same as above but allows rotation and flips

	//void cameraToRenderer();
	//void cameraClear();

	void set_zoom(double zoom);
	
	///double angle;

private:
	Vector2 FOV;
};

