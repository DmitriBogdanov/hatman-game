#include "sprite.h"

#include "graphics.h" // access to rendering
#include "game.h" // access to timescale



// # Animation #
Animation::Animation(SDL_Texture* texture, const std::vector<AnimationFrame> &frames) :
	texture(texture),
	frames(frames)
{}

Animation::Animation(SDL_Texture* texture, std::vector<AnimationFrame> &&frames) :
	texture(texture),
	frames(std::move(frames))
{}

Animation::Animation(SDL_Texture* texture, const std::initializer_list<AnimationFrame> &frames) :
	texture(texture),
	frames(frames)
{}

Animation::Animation(SDL_Texture* texture, const srcRect &frame) :
	texture(texture),
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
	current_texture(nullptr),
	parent_position(parentPosition),
	centered(centered),
	overlay(overlay)
{}

void Sprite::update(Milliseconds elapsedTime) {} // does nothing

void Sprite::draw() const {
	if (!this->current_texture) {
		return; /// TESTING, REMOVE LATER
	}

	dstRect destRect = make_dstRect(
		this->parent_position.x, this->parent_position.y,
		this->current_source_rect.w, this->current_source_rect.h,
		this->centered
	);

	SDL_SetTextureColorMod(this->current_texture, this->color_mod.r, this->color_mod.g, this->color_mod.b);
	SDL_SetTextureAlphaMod(this->current_texture, this->color_mod.alpha);

	// Draw to correct backbuffer
	if (this->overlay) {
		Graphics::ACCESS->gui->textureToGUIEx(this->current_texture, &this->current_source_rect, &destRect, this->angle, this->flip);
	}
	else {
		Graphics::ACCESS->camera->textureToCameraEx(this->current_texture, &this->current_source_rect, &destRect, this->angle, this->flip);
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
	SDL_Texture *texture
) :
	Sprite(parentPosition, centered, overlay)
{
	// Deduce texture size
	int textureWidth;
	int textureHeight;
	SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

	this->current_texture = texture;
	this->current_source_rect = { 0, 0, textureWidth, textureHeight };
}

StaticSprite::StaticSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	SDL_Texture *texture,
	srcRect sourceRect // source rect on the current_texture
) :
	Sprite(parentPosition, centered, overlay)
{
	this->current_texture = texture;
	this->current_source_rect = sourceRect;
}

void StaticSprite::update(Milliseconds elapsedTime) {}



// # AnimatedSprite #
AnimatedSprite::AnimatedSprite(
	const Vector2d &parentPosition,
	bool centered,
	bool overlay,
	Animation &&animation
) : 
	Sprite(parentPosition, centered, overlay),
	animation(std::move(animation))
{
	this->current_texture = this->animation.texture;
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

	this->current_source_rect = this->animation.frames.at(this->frame_index).rect;
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

	this->current_texture = this->animation_current->texture;
	this->current_source_rect = this->animation_current->frames.front().rect;
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

	this->current_source_rect =this->animation_current->frames.at(this->frame_index).rect;
}