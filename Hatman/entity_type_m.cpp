#include "entity_type_m.h"

#include "graphics.h" // access to texture loading
#include "game.h" // access to game state
#include "item_unique.h" // item classes (to assign loot to entities)
#include "controls.h" // access to control keys
#include "globalconsts.hpp" // tile size



using namespace ntt;

// # Creature #
m_type::Creature::Creature(const Vector2d &position) :
	Entity(position),
	creature_is_alive(true)
{}

TypeId m_type::Creature::type_id() const { return TypeId::CREATURE; }

bool m_type::Creature::update(Milliseconds elapsedTime) {
	if (!Entity::update(elapsedTime)) return false;
	
	this->sprite->flip = (this->orientation == Orientation::RIGHT) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

	this->_kill_if_out_of_bounds();

	if (this->health->dead() && creature_is_alive) {
		this->deathTransition();
	}

	return true;
}

// Utilities
bool m_type::Creature::_has_ground_in_front() const {
	return
		this->orientation == Orientation::LEFT
		? this->solid->is_grounded_at_left
		: this->solid->is_grounded_at_right;
}

// State
int m_type::Creature::state_get() const {
	return this->state;
}

bool m_type::Creature::state_isLocked() const {
	return !this->state_lock_timer.finished();
}

bool m_type::Creature::state_isUnlocked() const {
	return this->state_lock_timer.finished();
}

void m_type::Creature::state_lock(Milliseconds duration) {
	this->state_lock_timer.start(duration);
}

void m_type::Creature::state_unlock() {
	this->state_lock_timer.stop();
}

void m_type::Creature::_init_sprite(const std::string &folder, std::initializer_list<std::string> animationNames) {
	this->_parse_controllable_sprite(folder, animationNames);

	this->_sprite = static_cast<ControllableSprite*>(this->sprite.get());
}

// Module inits
void m_type::Creature::_init_solid(const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		flags,
		mass,
		friction
		);
}

void m_type::Creature::_init_health(Faction faction, uint maxHp, sint regen, sint physRes, sint magicRes, sint chaosRes) {
	this->health = std::make_unique<Health>(faction, maxHp, regen, physRes, magicRes, chaosRes);
}

void m_type::Creature::deathTransition() {
	this->creature_is_alive = false;
}

void m_type::Creature::_kill_if_out_of_bounds() {
	const auto hitbox = this->solid->getHitbox();
	const double levelHeight = Game::READ->level->getSizeY() * natural::TILE_SIZE;

	if (hitbox.getBottom() > levelHeight) this->health->instakill();
}



// # Enemy #
m_type::Enemy::Enemy(const Vector2d &position) :
	Creature(position),
	target(nullptr),
	target_relative_pos(0, 0),
	death_delay(0.)
{
	this->state_change(this->default_deaggroed_state);
}

TypeId m_type::Enemy::type_id() const { return TypeId::ENEMY; }

bool m_type::Enemy::update(Milliseconds elapsedTime) {
	if (!Creature::update(elapsedTime) || !this->creature_is_alive) return false;

	// Check validity of the target (aka target still alive/exists)
	if (this->target && !Game::ACCESS->level->entities_type[TypeId::CREATURE].count(this->target)) {
		this->target = nullptr;
		this->state_change(this->default_deaggroed_state);
		this->deaggroTransition();
	}

	// If target present check whether you should deaggro
	if (this->target) {
		this->target_relative_pos = this->target->position - this->position;

		this->update_when_aggroed(elapsedTime);

		if (this->deaggroCondition(this->target)) {
			this->target = nullptr;
			this->state_change(this->default_deaggroed_state);
			this->deaggroTransition();
		}
	}
	// If target NOT present search for target
	else {
		this->update_when_deaggroed(elapsedTime);

		for (const auto& entity : Game::ACCESS->level->entities_type[TypeId::CREATURE]) {
			const auto creature = static_cast<Creature*>(entity); // we 100% know that it's creature

			if (creature->health->faction != this->health->faction && this->aggroCondition(creature)) {
				this->target = creature;
				this->state_change(this->default_aggroed_state);
				this->aggroTransition();
			}
		}
	}

	return true;
}

void m_type::Enemy::draw() const {
	Creature::draw();

	if (this->healthbar_display && this->creature_is_alive) this->healthbar_display->draw();
}

void m_type::Enemy::aggroTransition() {}

void m_type::Enemy::deaggroTransition() {}

// Effects
void m_type::Enemy::deathTransition() {
	Creature::deathTransition();

	this->mark_for_erase(this->death_delay);
}

// Module inits
void m_type::Enemy::_optinit_healthbar_display(const Vector2d &parentPosition, const Health &parentHealth, const Vector2d &bottomCenterpointAlignment) {
	this->healthbar_display = std::make_unique<HealthbarDisplay>(parentPosition, parentHealth, bottomCenterpointAlignment);
}

void m_type::Enemy::_optinit_boss_healthbar_display(const Health &parentHealth, const std::string &bossTitle) {
	this->healthbar_display = std::make_unique<BossHealthbarDisplay>(parentHealth, bossTitle);
}

void m_type::Enemy::_optinit_death_delay(Milliseconds delay) {
	this->death_delay = delay;
}



// # ItemEntity #
namespace ItemEntity_consts {
	constexpr double DEFAULT_MASS = 40.;
	constexpr double DEFAULT_FRICTION = 0.3;
}

m_type::ItemEntity::ItemEntity(const Vector2d &position) :
	Entity(position)
{}

TypeId m_type::ItemEntity::type_id() const { return TypeId::ITEM_ENTITY; }

bool m_type::ItemEntity::update(Milliseconds elapsedTime) {
	if (!Entity::update(elapsedTime)) return false;

	if (this->checkActivation()) {
		this->activate();

		if (this->checkTrigger())
			this->trigger();
	}

	return true;
}

// Checks
bool m_type::ItemEntity::checkActivation() const {
	if (this->solid && this->solid->getHitbox().overlapsWithRect(Game::ACCESS->level->player->solid->getHitbox()))
		return true;

	return false;
}

bool m_type::ItemEntity::checkTrigger() const {
	bool res = false;

	if (Game::ACCESS->input.key_pressed(Controls::READ->USE)) {
		res = true;
		/// Check if player has free inventory space
	}

	return res;
}

// Actions
void m_type::ItemEntity::activate() {
	// No default effects
}

void m_type::ItemEntity::trigger() {
	Game::ACCESS->play_sound("item_pick_up.wav", 1.2);

	const auto item = items::make_item(this->name);
	Game::ACCESS->level->player->inventory.addItem(*item);

	this->mark_for_erase();
}

// Module inits
void m_type::ItemEntity::_init_sprite(bool animated, const std::string &folder, const std::string &filename) {
	if (animated)
		this->_parse_animated_sprite(folder, filename);
	else
		this->_parse_static_sprite(folder, filename);
}

void m_type::ItemEntity::_init_solid(const Vector2d &hitboxSize) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
		ItemEntity_consts::DEFAULT_MASS,
		ItemEntity_consts::DEFAULT_FRICTION
		);
}

// Member inits
void m_type::ItemEntity::_init_name(const std::string &name) {
	this->name = name;
}



// # Destructible #
namespace Destructible_consts {
	constexpr double DEFAULT_MASS = 80.;
	constexpr double DEFAULT_FRICTION = 0.6;
}

m_type::Destructible::Destructible(const Vector2d &position) :
	Entity(position)
{}

TypeId m_type::Destructible::type_id() const { return TypeId::DESTRUCTIBLE; }

bool m_type::Destructible::update(Milliseconds elapsedTime) {
	if (!Entity::update(elapsedTime)) return false;

	if (this->health->dead()) {
		// Trigger effect/animation if not triggered yet
		if (!this->effect_triggered) {
			this->effect();
			this->effect_triggered = true;

			this->timer.start(this->erasion_delay);
		}

		// Erase entity after delay
		if (this->timer.finished()) {
			this->mark_for_erase();
		}
	}

	return true;
}

// Effects
void m_type::Destructible::effect() {} // nothing by default

// Module inits
void m_type::Destructible::_init_sprite(const std::string &folder, std::initializer_list<std::string> animationNames) {
	this->_parse_controllable_sprite(folder, animationNames);

	this->_sprite = static_cast<ControllableSprite*>(this->sprite.get());
}

void m_type::Destructible::_init_solid(const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		flags,
		mass,
		friction
		);
}

void m_type::Destructible::_init_health(Faction faction, uint maxHp, sint regen, sint physRes, sint magicRes, sint chaosRes) {
	this->health = std::make_unique<Health>(faction, maxHp, regen, physRes, magicRes, chaosRes);
}

// Member inits
void m_type::Destructible::_init_delay(Milliseconds erasionDelay) {
	this->erasion_delay = erasionDelay;
}