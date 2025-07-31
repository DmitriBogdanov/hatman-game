#include "entity/base.h"

#include "firstparty/UTL/json.hpp" // parsing JSON
#include <fstream>                 // reading files

#include "graphics/graphics.h"   // access to texture loading
#include "utility/filepaths.hpp" // path to textures



using namespace ntt;

// # Entity #
Entity::Entity(const Vector2d& position) : position(position) {}

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
    if (!this->erase_timer) { this->erase_timer = std::make_unique<Timer>(); }
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
void Entity::parse_static_sprite(const std::string& entityName, const std::string& textureName) {
    this->sprite = std::make_unique<StaticSprite>(
        this->position, true, false,
        Graphics::ACCESS->get_texture(paths::TEXTURES_ENTITIES + entityName + "/" + textureName + ".png"));
}

void Entity::parse_animated_sprite(const std::string& entityName, const std::string& animationName) {
    this->sprite = std::make_unique<AnimatedSprite>(
        this->position, true, false, parse_animation(paths::TEXTURES_ENTITIES + entityName + "/" + animationName));
}

void Entity::parse_controllable_sprite(const std::string&                 entityName,
                                       std::initializer_list<std::string> animationNames) {
    auto controllableSprite = std::make_unique<ControllableSprite>(this->position, true, false);

    bool defaultAnimationNotSet = true;

    for (auto& name : animationNames) {
        controllableSprite->animation_add(name, parse_animation(paths::TEXTURES_ENTITIES + entityName + "/" + name));

        if (defaultAnimationNotSet && name == default_animation_name) {
            controllableSprite->animation_play(default_animation_name, true);
            defaultAnimationNotSet = false;
        }
    }

    this->sprite = std::move(controllableSprite);
}



/// THINK ABOUT CACHING ANIMATIONS
Animation parse_animation(const std::string& path) {
    const utl::json::Node JSON = utl::json::from_file(path + ".json");

    // Parse texture
    sf::Texture& texture = Graphics::ACCESS->get_texture(path + ".png");

    // Parse frames
    std::vector<AnimationFrame> frames;

    for (const auto& node : JSON["frames"].get_array()) {
        frames.push_back(AnimationFrame{make_src_rect(node["frame"]["x"].get_number(), node["frame"]["y"].get_number(),
                                                      node["frame"]["w"].get_number(), node["frame"]["h"].get_number()),
                                        node["duration"].get_number()});
    }

    return Animation(texture, std::move(frames));
}