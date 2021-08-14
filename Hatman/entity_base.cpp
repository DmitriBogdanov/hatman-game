#include "entity_base.h"

#include <fstream> // reading files
#include "nlohmann_external.hpp" // parsing JSON

#include "graphics.h" // access to texture loading
#include "filepaths.hpp" // path to textures



using namespace ntt;

// # Entity #
Entity::Entity(const Vector2d &position) :
	position(position),
	sprite(nullptr),
	solid(nullptr),
	health(nullptr),
	enabled(true),
	erase_timer(nullptr)
{}

bool Entity::update(Milliseconds elapsedTime) {
	if (!this->enabled) return false;

	if (this->sprite) { this->sprite->update(elapsedTime); }
	if (this->solid) { this->solid->update(elapsedTime); }
	if (this->health) { this->health->update(elapsedTime); }

	return true;
}

void Entity::draw() const {
	if (!this->enabled) return;

	if (this->sprite) { this->sprite->draw(); }
}

void Entity::mark_for_erase() {
	if (!this->erase_timer) {
		this->erase_timer = std::make_unique<Timer>();
	}
}

void Entity::mark_for_erase(Milliseconds delay) {
	if (!delay) this->marked_for_erase();

	if (!this->erase_timer) {
		this->erase_timer = std::make_unique<Timer>();
		this->erase_timer->start(delay);
	}
	
}

bool Entity::marked_for_erase() const {
	return this->erase_timer && this->erase_timer->finished(); // doesn't request ->finished() if timer doesn't exist
}

// Methods for parsing entity sprites from files
void Entity::_parse_static_sprite(const std::string &entityName, const std::string &textureName) {
	this->sprite = std::make_unique<StaticSprite>(
		this->position,
		true,
		false,
		Graphics::ACCESS->getTexture(PATH_TEXTURES_ENTITIES + entityName + "/" + textureName + ".png")
		);
}

void Entity::_parse_animated_sprite(const std::string &entityName, const std::string &animationName) {
	this->sprite = std::make_unique<AnimatedSprite>(
		this->position,
		true,
		false,
		_parse_animation(PATH_TEXTURES_ENTITIES + entityName + "/" + animationName)
		);
}

void Entity::_parse_controllable_sprite(const std::string &entityName, std::initializer_list<std::string> animationNames) {
	auto controllableSprite = std::make_unique<ControllableSprite>(
		this->position,
		true,
		false
		);

	bool defaultAnimationNotSet = true;

	for (auto &name : animationNames) {
		controllableSprite->animation_add(name, _parse_animation(PATH_TEXTURES_ENTITIES + entityName + "/" + name));
	
		if (defaultAnimationNotSet && name == DEFAULT_ANIMATION_NAME) {
			controllableSprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			defaultAnimationNotSet = false;
		}
	}

	this->sprite = std::move(controllableSprite);
}



/// THINK ABOUT CACHING ANIMATIONS
Animation _parse_animation(const std::string &path) {
	std::ifstream ifStream(path + ".json");
	nlohmann::json JSON = nlohmann::json::parse(ifStream);

	// Parse texture
	SDL_Texture* texture = Graphics::ACCESS->getTexture(path + ".png");

	// Parse frames
	std::vector<AnimationFrame> frames;

	for (const auto &node : JSON["frames"]) {
		frames.push_back(AnimationFrame{
			make_srcRect(
				node["frame"]["x"].get<int>(),
				node["frame"]["y"].get<int>(),
				node["frame"]["w"].get<int>(),
				node["frame"]["h"].get<int>()),
			node["duration"].get<double>()
			});
	}

	return Animation(texture, std::move(frames));
}