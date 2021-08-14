#include "camera.h"

#include "graphics.h" // access to rendering
#include "globalconsts.hpp" // natural consts



// # Camera #
Camera::Camera(const Vector2d &position) :
	position(position),
	zoom(1),
	angle(0)
{
	this->standard_FOV = natural::DIMENSIONS;
	this->scaled_standard_FOV = Vector2(Graphics::READ->width(), Graphics::READ->height());

	this->backbuffer_size = (scaled_standard_FOV + Vector2(this->MARGIN, this->MARGIN)) * 2;

	this->backbuffer = SDL_CreateTexture(
		Graphics::ACCESS->getRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
		this->backbuffer_size.x, this->backbuffer_size.y
	);

	SDL_SetTextureBlendMode(this->backbuffer, SDL_BLENDMODE_BLEND); // necessary for proper blending of of transparent parts
}
Camera::~Camera() {
	SDL_DestroyTexture(this->backbuffer);
}

// FOV
Vector2d Camera::getFOV_Corner() const {
	return this->position - this->getFOV_Size() / 2.;
}

Vector2d Camera::getFOV_Size() const {
	return this->standard_FOV * this->zoom;
}

dRect Camera::getFOV_Rect() const {
	return dRect(this->position, this->getFOV_Size(), true);
}

// Position conversions
Vector2d Camera::get_ScreenPos_from_LevelPos(const Vector2d &levelPos) const {
	return levelPos - this->getFOV_Corner();
}

Vector2d Camera::get_LevelPos_from_ScreenPos(const Vector2d &screenPos) const {
	return screenPos + this->getFOV_Corner();
}

void Camera::textureToCamera(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	const double scalingFactor = Graphics::READ->scaling_factor();
	const Vector2d scaledPosition = this->position * scalingFactor;

	const Vector2 cameraCornerPos =
		(scaledPosition.toVector2() - this->backbuffer_size / 2 + Vector2(this->MARGIN, this->MARGIN));
	// position of top-left corner of the camera with standard zoom

	SDL_Rect backbufferDestRect = {
		static_cast<int>(this->MARGIN + destRect->x * scalingFactor - cameraCornerPos.x),
		static_cast<int>(this->MARGIN + destRect->y * scalingFactor - cameraCornerPos.y),
		static_cast<int>(destRect->w * scalingFactor),
		static_cast<int>(destRect->h * scalingFactor)
	};

	SDL_RenderCopy(Graphics::ACCESS->getRenderer(), texture, sourceRect, &backbufferDestRect);
}
void Camera::textureToCameraEx(SDL_Texture* texture, const srcRect* sourceRect, const dstRect* destRect, double angle, SDL_RendererFlip flip) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	const double scalingFactor = Graphics::READ->scaling_factor();
	const Vector2d scaledPosition = this->position * scalingFactor;

	const Vector2 cameraCornerPos =
		scaledPosition.toVector2() - this->backbuffer_size / 2 + Vector2(this->MARGIN, this->MARGIN);
	// position of top-left corner of the camera with standard zoom

	SDL_Rect backbufferDestRect = {
		static_cast<int>(this->MARGIN + destRect->x * scalingFactor - cameraCornerPos.x),
		static_cast<int>(this->MARGIN + destRect->y * scalingFactor - cameraCornerPos.y),
		static_cast<int>(destRect->w * scalingFactor),
		static_cast<int>(destRect->h * scalingFactor)
	};

	SDL_RenderCopyEx(Graphics::ACCESS->getRenderer(), texture, sourceRect, &backbufferDestRect, angle, NULL, flip);
}

void Camera::cameraToRenderer() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // give target back to the renderer

	const Vector2 sourceRectCenter = this->backbuffer_size / 2;
	const Vector2 sourceRectDimensions = this->scaled_standard_FOV * zoom + Vector2(this->MARGIN, this->MARGIN) * 2 * zoom;

	SDL_Rect sourceRect = dRect(sourceRectCenter, sourceRectDimensions, true).to_SDL_Rect();
	SDL_Rect destRect = {
		-this->MARGIN, -this->MARGIN,
		Graphics::READ->width() + 2 * this->MARGIN, Graphics::READ->height() + 2 * this->MARGIN
	}; // account for different backbuffer including marign 

	Graphics::ACCESS->copyTextureToRendererEx(this->backbuffer, &sourceRect, &destRect, this->angle);
}
void Camera::cameraClear() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), this->backbuffer); // target backbuffer for rendering

	SDL_RenderClear(Graphics::ACCESS->getRenderer());
}