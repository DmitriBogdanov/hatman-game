#include "modules/sprite.h"

#include "graphics/graphics.h" // access to rendering
#include "systems/game.h" // access to timescale



// # Animation #
Animation::Animation(sf::Texture &texture, const std::vector<AnimationFrame> &frames) :
	texture(&texture),
	frames(frames)
{}

Animation::Animation(sf::Texture &texture, std::vector<AnimationFrame> &&frames) :
	texture(&texture),
	frames(std::move(frames))
{}

Animation::Animation(sf::Texture &texture, std::initializer_list<AnimationFrame> frames) :
	texture(&texture),
	frames(frames)
{}

Animation::Animation(sf::Texture &texture, const srcRect &frame) :
	texture(&texture),
	frames({ AnimationFrame{frame, 0} })
{}

bool Animation::isSingleFrame() const {
	return (this->frames.size() == 1);
}

size_t Animation::lastIndex() const {
	return this->frames.size() - 1;
}



// # Sprite #
Sprite::Sprite(const Vector2d &parentPosition, bool centered, bool overlay) :
	parent_position(parentPosition),
	centered(centered),
	overlay(overlay)
{}

void Sprite::update([[maybe_unused]] Milliseconds elapsedTime) {} // does nothing

void Sprite::draw() {
	dstRect destRect = make_dstRect(
		this->parent_position.x, this->parent_position.y,
		this->current_sprite.getTextureRect().width, this->current_sprite.getTextureRect().height,
		this->centered
	);

	this->current_sprite.setColor(sf::Color(
		this->color_mod.r, this->color_mod.g, this->color_mod.b, this->color_mod.alpha
	));


	// Draw to correct coords
	this->current_sprite.setPosition(
		static_cast<float>(destRect.x),
		static_cast<float>(destRect.y)
	);

	/// Rotation can be implemented

	// IMPLEMENT FLIP
	switch (this->flip) {
	case Flip::HORIZONTAL:
		this->current_sprite.setOrigin(static_cast<float>(destRect.w), 0.f);
		this->current_sprite.setScale(-1.f, 1.f);
		break;
	case Flip::NONE:
		this->current_sprite.setOrigin(0.f, 0.f);
		this->current_sprite.setScale(1.f, 1.f);
		break;
		/// Vertical can be implemented in a similar fashion
	default:
		break;
	}

	if (this->overlay) {
		Graphics::ACCESS->gui->draw_sprite(this->current_sprite);
	}
	else {
		Graphics::ACCESS->camera->draw_sprite(this->current_sprite);
	}
}

void Sprite::setRotation(double radians) {
	this->angle = helpers::rad_to_degree(radians);
}

void Sprite::setRotationDegrees(double degrees) {
	this->angle = degrees;
}



// # StaticSprite #
StaticSprite::StaticSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	sf::Texture &texture
) :
	Sprite(parentPosition, centered, overlay)
{
	this->current_sprite.setTexture(texture);
}

StaticSprite::StaticSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	sf::Texture &texture,
	srcRect sourceRect // source rect on the current_texture
) :
	Sprite(parentPosition, centered, overlay)
{
	this->current_sprite.setTexture(texture);
	this->current_sprite.setTextureRect(sf::IntRect(
		sourceRect.x, sourceRect.y,
		sourceRect.w, sourceRect.h
	));
}

void StaticSprite::update([[maybe_unused]] Milliseconds elapsedTime) {}



// # AnimatedSprite #
AnimatedSprite::AnimatedSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	Animation &&animation
) : 
	Sprite(parentPosition, centered, overlay),
	animation(std::move(animation)),
	time_elapsed(0),
	frame_index(0)
{
	this->current_sprite.setTexture(*this->animation.texture);
}

void AnimatedSprite::update(Milliseconds elapsedTime) {
	if (!animation.isSingleFrame()) {
		this->time_elapsed += elapsedTime;

		const Milliseconds timeBeforeUpdate = animation.frames.at(this->frame_index).duration; // time current animation frame is supposed to be displayed

		if (this->time_elapsed > timeBeforeUpdate) {
			this->time_elapsed = 0.;

			if (this->frame_index < animation.frames.size() - 1) { // if current frame is not the last
				++this->frame_index;
			}
			else { // if current frame is the last
				this->frame_index = 0;
			}
		}
	}

	const auto &rect = this->animation.frames.at(this->frame_index).rect;

	this->current_sprite.setTextureRect(sf::IntRect(
		rect.x, rect.y,
		rect.w, rect.h
	));
}



// # ControllableSprite #
ControllableSprite::ControllableSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay
) :
	Sprite(parentPosition, centered, overlay),
	animation_current(nullptr),
	animation_current_looped(false),
	animation_current_finished(true),
	//animation_queued(nullptr),
	//animation_queued_looped(false),
	time_elapsed(0.),
	frame_index(0),
	timescale(1.)
{}

void ControllableSprite::animation_add(const std::string &name, Animation &&animation) {
	this->animations[name] = std::move(animation);
}

void ControllableSprite::animation_play(const std::string &name, bool loop) {
	this->animation_current = &this->animations.at(name);
	this->animation_current_looped = loop;
	this->animation_current_finished = false;

	this->time_elapsed = 0.;
	this->frame_index = 0;
	this->timescale = 1.;

	this->current_sprite.setTexture(*this->animation_current->texture);

	const auto &rect = this->animation_current->frames.front().rect;

	this->current_sprite.setTextureRect(sf::IntRect(
		rect.x, rect.y,
		rect.w, rect.h
	));
}

//void ControllableSprite::animation_queue(const std::string& name, bool loop) {
//	this->animation_queued = &this->animations.at(name);
//	this->animation_queued_looped = loop;
//}

bool ControllableSprite::animation_awaitEnd() {
	this->animation_current_looped = false;

	return this->animation_finished();
}

bool ControllableSprite::animation_rushToEnd(double timescale) {
	this->timescale = timescale;

	return this->animation_awaitEnd();
}

bool ControllableSprite::animation_finished() const {
	return this->animation_current_finished;
}

Milliseconds ControllableSprite::animation_duration(const std::string &name) const {
	Milliseconds total = 0.;

	for (const auto& frame : this->animations.at(name).frames) {
		total += frame.duration; /// Consider imbedding duration in animation itself
	}

	return total;
}

void ControllableSprite::update(Milliseconds elapsedTime) {
	//if (this->animation_queued && this->animation_finished()) {
	//	this->animation_current = this->animation_queued;
	//	this->animation_current_looped = this->animation_queued_looped;

	//	this->animation_queued = nullptr;
	//	// no need to reset 'this->animation_queued_looped', it's gonna reset when we queue next time

	//	this->time_elapsed = 0.;
	//	this->frame_index = 0;
	//}

	this->time_elapsed += elapsedTime * this->timescale;

	const Milliseconds timeBeforeUpdate = this->animation_current->frames.at(this->frame_index).duration; // time current animation frame is supposed to be displayed

	if (this->time_elapsed > timeBeforeUpdate) {
		this->time_elapsed = 0.;

		// Current frame is NOT the last
		if (this->frame_index < this->animation_current->lastIndex()) { // if current frame is not the last
			++this->frame_index;
		}
		// Current frame is the last
		else {
			if (this->animation_current_looped) { this->frame_index = 0; }
			else { this->animation_current_finished = true; }
			// non-looped animations are left at the last frame
		}
	}

	const auto &rect = this->animation_current->frames.at(this->frame_index).rect;

	this->current_sprite.setTextureRect(sf::IntRect(
		rect.x, rect.y,
		rect.w, rect.h
	));
}