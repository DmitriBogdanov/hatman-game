// ____________________ HEADER SUMMARY ____________________

// Polymorphic base class for all the entities.

// ____________________ IMPLEMENTATION ____________________

#pragma once

#include <memory> // types

#include "modules/solid.h"    // module 'SolidRectangle'
#include "modules/sprite.h"   // module 'ControllableSprite'
#include "modules/stats.h"    // module 'Health'
#include "utility/geometry.h" // types



// ntt::
// - 'ntt' stands for 'Entity'
// - Contains all entity-related classes
namespace ntt {

// # TypeId #
// - Used to determine entity types runtime
// - Should contain ALL entity types
enum class TypeId {
    // m_type::
    CREATURE,
    ENEMY,
    ITEM_ENTITY,
    DESTRUCTIBLE,
    // s_type::
    PROJECTILE,
    PARTICLE,
    // other
    PLAYER
    /// New entity types go there
};


// # Entity #
// - ABSTRACT
// - Base class for all entity types
// - Contains a number of optional modules
class Entity {
public:
    Entity() = delete;

    Entity(const Vector2d& position);

    virtual ~Entity() = default;

    virtual TypeId type_id() const = 0; // MUST be overriden for ALL derived classes

    virtual bool update(Milliseconds elapsedTime); // updates all logic, returns this->enabled
    virtual void draw() const;                     // draws a correct frame of current animation

    void mark_for_erase();                   // instantly disables entity and marks for erasion
    void mark_for_erase(Milliseconds delay); // marks entity for erasion after a delay

    bool marked_for_erase() const; // returns whether entity should be erased

    Vector2d position{}; // position in a level

    std::unique_ptr<Sprite>         sprite{};
    std::unique_ptr<SolidRectangle> solid{};
    std::unique_ptr<Health>         health{};

    bool enabled = true; // entity doesn't update/draw if disabled

protected:
    std::unique_ptr<Timer> erase_timer = nullptr; // if exists and finished => entity should be erased

    // Methods for parsing entity sprites from files
    void parse_static_sprite(const std::string& folder, const std::string& filename = default_animation_name);
    void parse_animated_sprite(const std::string& folder, const std::string& filename = default_animation_name);
    void parse_controllable_sprite(const std::string& folder, std::initializer_list<std::string> animationNames);
    // if animation with default animation name is found it's set as current
    // 'folder' refers to folder in ./content/textures/entities/, no need to write full path
    // 'filename' does NOT include file extension
};

} // namespace ntt

Animation parse_animation(const std::string& path);