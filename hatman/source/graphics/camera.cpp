#include "graphics/camera.h"

#include <iostream>

#include "graphics/graphics.h" // access to rendering
#include "utility/globalconsts.hpp" // natural consts



// # Camera #
Camera::Camera(const Vector2d &position) :
	position(position)
{
	std::cout << "Creating camera graphics...\n";
	
	this->set_zoom(natural::ZOOM);
}


// FOV
Vector2d Camera::get_FOV_corner() const {
	return this->position - this->get_FOV_size() / 2.;
}

Vector2d Camera::get_FOV_size() const {
	return this->FOV;
}

dRect Camera::get_FOV_rect() const {
	return dRect(this->position, this->get_FOV_size(), true);
}

// Position conversions
Vector2d Camera::get_ScreenPos_from_LevelPos(const Vector2d &levelPos) const {
	return levelPos - this->get_FOV_corner();
}

Vector2d Camera::get_LevelPos_from_ScreenPos(const Vector2d &screenPos) const {
	return screenPos + this->get_FOV_corner();
}

void Camera::set_zoom(double zoom) {
	this->FOV = (natural::DIMENSIONS * zoom).to_Vector2();
}

void Camera::draw_sprite(sf::Sprite &sprite) {
	// Candle sprite position
	const sf::Vector2f minusCameraCornerPos(
		static_cast<float>(this->FOV.x / 2. - this->position.x),
		static_cast<float>(this->FOV.y / 2. - this->position.y)
	); // position of top-left corner of the camera

	sprite.move(minusCameraCornerPos);

	// Set up camera view
	const auto FOV_size = Graphics::READ->camera->get_FOV_size();

	sf::View camera_view;
	camera_view.setCenter(
		static_cast<float>(FOV_size.x / 2.),
		static_cast<float>(FOV_size.y / 2.)
	);
	camera_view.setSize(
		static_cast<float>(FOV_size.x),
		static_cast<float>(FOV_size.y)
	);

	// !!! Fix for the rounding issue that causes vertical black lines !!!
	// !!! to sometimes appear on certain camera coords                !!!
	const float scaling_factor = static_cast<float>(Graphics::READ->scaling_factor());

	sf::Vector2f oldPosition = sprite.getPosition();
	sprite.setPosition(std::floor(oldPosition.x + .5f / scaling_factor), std::floor(oldPosition.y + .5f / scaling_factor));

	Graphics::ACCESS->window.setView(camera_view);

	// Draw
	Graphics::ACCESS->window_draw_sprite(sprite);
}