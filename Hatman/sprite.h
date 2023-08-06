#pragma once

/* Contains modules: 'StaticSprite', 'AnimatedSprite', 'ControllableSprite' */

#include <SDL.h> // 'SDL_texture' and related types
#include <SFML/Graphics.hpp>

#include <string> // related type
#include <initializer_list> // related type
#include <unordered_map> // related type


#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "color.hpp" // 'RGBColor' type

#define DEFAULT_ANIMATION_NAME "default"



// # AnimationFrame #
struct AnimationFrame {
	srcRect rect;
	Milliseconds duration;
};



// # Animation #
// - Contains frames of an animation and delays between these frames
// - Does NOT handle time recording, updating of frame indexes and etc
struct Animation {
	Animation() = default;

	Animation(sf::Texture &texture, const std::vector<AnimationFrame> &frames);
	Animation(sf::Texture &texture, std::vector<AnimationFrame> &&frames); // move semantics
	Animation(sf::Texture &texture, std::initializer_list<AnimationFrame> frames);
	Animation(sf::Texture &texture, const srcRect &frame); // creates single-frame animation

	bool isSingleFrame() const; // if "Animation" has a single frame returns true

	size_t lastIndex() const;

	sf::Texture* texture;
	std::vector<AnimationFrame> frames; // holds source rectangles and display time of all frames of animation
};



// # Sprite #
// - ABSTRACT
class Sprite {
public:
	Sprite() = delete;

	Sprite(const Vector2d &parentPosition, bool centered, bool overlay);

	virtual ~Sprite() = default;

	virtual void update(Milliseconds elapsedTime); // does nothing
	void draw(); // draws from current_source_rect to dest_rect

	void setRotation(double radians);
	void setRotationDegrees(double degrees);
	
	Vector2d alignment;
	SDL_RendererFlip flip = SDL_FLIP_NONE; // change to flip textures
	RGBColor color_mod;
protected:
	sf::Sprite current_sprite;

	///sf::Texture* current_texture; // texture used by 'draw()'
	///srcRect current_source_rect; // source rect used by 'draw()'

	const Vector2d &parent_position; // position of the object module is attached to
	bool centered; // decides if sprite is centered around parent_position
	bool overlay; // decides if sprite is rendered to GUI or to Camera

	double angle = 0.; // rotation in DEGREES
};



// # StaticSprite #
// - Sprite with no animation
class StaticSprite : public Sprite {
public:
	StaticSprite() = delete;

	StaticSprite(
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		sf::Texture &texture
		// if source rect is not specified it's assumed to be the whole texture
	);

	StaticSprite(
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		sf::Texture &texture,
		srcRect sourceRect // source rect on the current_texture
	);

	void update(Milliseconds elapsedTime) override; // does nothing
};



// # AnimatedSprite #
// - Sprite with a single looped animation
class AnimatedSprite : public Sprite {
public:
	AnimatedSprite() = delete;

	AnimatedSprite(
		const Vector2d &parentPosition,
		bool centered,
		bool overlay,
		Animation &&animation
	);

	void update(Milliseconds elapsedTime) override;

private:
	Animation animation; // holds source rectangles and display time of all frames of animation

	Milliseconds time_elapsed;
	size_t frame_index; // used for updating animations
};



// # ControllableSprite #
// - Sprite with a number of animations
// - Animations can be added to the sprite
// - Animations can be played upon request (once or in a loop)
class ControllableSprite : public Sprite {
public:
	ControllableSprite() = delete;

	ControllableSprite(
		const Vector2d &parentPosition,
		bool centered,
		bool overlay
	); // takes enitity current_texture

	void animation_add(const std::string &name, Animation &&animation);

	void animation_play(const std::string &name, bool loop = false);

	/// I question whether it's a needed functionality
	///void animation_queue(const std::string& name, bool loop = false); // only 1 animation can be queued at once

	bool animation_awaitEnd(); // disables loop and returnes whether animation is finished
	bool animation_rushToEnd(double timescale); // same as above but speeds up the animation untill it ends

	bool animation_finished() const;

	Milliseconds animation_duration(const std::string &name) const; // returns full duration of an animation

	void update(Milliseconds elapsedTime) override;

private:
	std::unordered_map<std::string, Animation> animations; // holds all spites (animated or not) for the entity

	Animation* animation_current;
	bool animation_current_looped;

	bool animation_current_finished;

	///Animation* animation_queued;
	///bool animation_queued_looped;

	Milliseconds time_elapsed;
	size_t frame_index; // used for updating animations	


	double timescale;
};
	