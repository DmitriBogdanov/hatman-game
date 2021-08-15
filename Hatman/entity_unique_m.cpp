#include "entity_unique_m.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)

#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "entity_unique_s.h" // particles and projectiles
#include "ct_math.hpp" // for compile-time math



using namespace ntt;

/* ### CONTROLLERS ### */

typedef std::function<std::unique_ptr<Entity>(const Vector2d&)> make_derived_ptr;

// std::make_unique() wrapper
template<class UniqueEntity>
std::unique_ptr<UniqueEntity> make_derived(const Vector2d &position) {
	return std::make_unique<UniqueEntity>(position);
}

// !!! NAMES !!!
const std::unordered_map<std::string, make_derived_ptr> ENTITY_MAKERS = {
	{"item-brass_relic", make_derived<m::item_entity::BrassRelic>},
	{"item-paper", make_derived<m::item_entity::Paper>},
	{"destructible-tnt", make_derived<m::destructible::TNT>},
	{"enemy-sludge", make_derived<m::enemy::Sludge>},
	{"enemy-skeleton_halberd", make_derived<m::enemy::SkeletonHalberd>}
	/// new entities go there
};

std::unique_ptr<Entity> m::make_entity(const std::string &type, const std::string &name, const Vector2d &position) {
	return ENTITY_MAKERS.at(type + '-' + name)(position); // type and name are joined into a single string
}



/* ### enemy:: ### */

// # Sludge #
namespace Sludge_consts {
	// Physics
	constexpr double MASS = 120.;
	constexpr double FRICTION = 0.6;

	// Stats
	constexpr Faction FACTION = Faction::UNDEAD;
	constexpr uint MAX_HP = 1900;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 50;
	constexpr sint MAGIC_RES = 0;
	constexpr sint DOT_RES = 60;

	constexpr Milliseconds ATTACK_CD = 550.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 150., 0., 50., 0.);
	constexpr double ATTACK_KNOCKBACK_X = 200. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 150. * 100.;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 30.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 30.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(2.);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(4.);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.8);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(2.);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 60.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 60.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Death
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 200.;
	constexpr double AGGRO_RANGE_Y = 40.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 250.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(8.);
}

m::enemy::Sludge::Sludge(const Vector2d &position) :
	Enemy(position)
{
	// Init modules
	this->_init_sprite("sludge", { DEFAULT_ANIMATION_NAME });

	this->_init_solid(
		Vector2d(14., 13.),
		SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
		Sludge_consts::MASS,
		Sludge_consts::FRICTION
	);

	this->_init_health(
		Sludge_consts::FACTION,
		Sludge_consts::MAX_HP,
		Sludge_consts::REGEN,
		Sludge_consts::PHYS_RES,
		Sludge_consts::MAGIC_RES,
		Sludge_consts::DOT_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::Sludge::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Sludge_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Sludge_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Sludge::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Sludge_consts::DEAGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) > Sludge_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Sludge::update_when_aggroed(Milliseconds elapsedTime) {
	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > 10.) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;
		}

		this->solid->applyForceTillMaxSpeed_Horizontal(
			Sludge_consts::MOVEMENT_FORCE_CHASE,
			Sludge_consts::MOVEMENT_SPEED_CHASE,
			this->orientation
		);
		
		// Transition
		if (hitboxOverlapArea > Sludge_consts::HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_tryChange(State::ATTACK);
		}

		break;

	case State::ATTACK:
		if (this->attack_cd.finished()) {
			this->target->health->applyDamage(Sludge_consts::ATTACK_DAMAGE);
			this->target->solid->addImpulse_Horizontal(Sludge_consts::ATTACK_KNOCKBACK_X * sign);
			this->target->solid->addImpulse_Up(Sludge_consts::ATTACK_KNOCKBACK_Y);

			this->attack_cd.start(Sludge_consts::ATTACK_CD);
		}

		// Transition
		if (hitboxOverlapArea < Sludge_consts::HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_tryChange(State::CHASE);
		}

		break;

	default:
		break;
	}
}

void m::enemy::Sludge::update_when_deaggroed(Milliseconds elapsedTime) {
	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::WANDER_STAND:
		// Transition
		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

				this->state_change(State::WANDER_MOVE);
				this->state_lock(rand_double(Sludge_consts::WANDER_MOVE_TIMER_MIN, Sludge_consts::WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->state_lock(rand_double(Sludge_consts::WANDER_STAND_TIMER_MIN, Sludge_consts::WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	case State::WANDER_MOVE:
		this->solid->applyForceTillMaxSpeed_Horizontal(
			Sludge_consts::MOVEMENT_FORCE_WANDER,
			Sludge_consts::MOVEMENT_SPEED_WANDER,
			this->orientation
		);

		// Transition
		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

				this->state_lock(rand_double(Sludge_consts::WANDER_MOVE_TIMER_MIN, Sludge_consts::WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->state_change(State::WANDER_STAND);
				this->state_lock(rand_double(Sludge_consts::WANDER_STAND_TIMER_MIN, Sludge_consts::WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	default:
		break;
	}
}

void m::enemy::Sludge::deathTransition() {
	Enemy::deathTransition();

	for (int i = 0; i < 16; ++i) {
		Game::ACCESS->level.spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-300., 300.), rand_double(-250., 0.)),
			colors::SH_BLACK,
			rand_double(Sludge_consts::PARTICLE_DURATION_MIN, Sludge_consts::PARTICLE_DURATION_MAX)
			));
	}
}



// # SkeletonHalberd #
namespace SkeletonHalberd_consts {
	// Physics
	constexpr double MASS = 140.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::UNDEAD;
	constexpr uint MAX_HP = 1300;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 20;
	constexpr sint MAGIC_RES = 20;
	constexpr sint DOT_RES = 20;

	constexpr Milliseconds ATTACK_CD = 2000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 350., 0., 0., 0.);
	constexpr double ATTACK_KNOCKBACK_X = 250. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 40.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 60.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(0.1);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(0.3);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.1);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(0.3);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 80.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 300.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 280.;
	constexpr double AGGRO_RANGE_Y = 40.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 250.;

	constexpr double ATTACK_RANGE_FRONT = 30.;
	constexpr double ATTACK_RANGE_BACK = 2.;
	constexpr double ATTACK_RANGE_UP = 20.;
	constexpr double ATTACK_RANGE_DOWN = 12.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(2.);

	// Death
	constexpr Milliseconds CORPSE_LIFETIME_MIN = sec_to_ms(1.);
	constexpr Milliseconds CORPSE_LIFETIME_MAX = sec_to_ms(2.);
}

m::enemy::SkeletonHalberd::SkeletonHalberd(const Vector2d &position) :
	Enemy(position)
{
	// Init modules
	this->_init_sprite("skeleton_halberd", { DEFAULT_ANIMATION_NAME, "move", "attack_windup", "attack_recover", "death" });

	this->_init_solid(
		Vector2d(10., 24.),
		SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
		SkeletonHalberd_consts::MASS,
		SkeletonHalberd_consts::FRICTION
	);

	this->_init_health(
		SkeletonHalberd_consts::FACTION,
		SkeletonHalberd_consts::MAX_HP,
		SkeletonHalberd_consts::REGEN,
		SkeletonHalberd_consts::PHYS_RES,
		SkeletonHalberd_consts::MAGIC_RES,
		SkeletonHalberd_consts::DOT_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));

	this->_optinit_death_delay(
		this->_sprite->animation_duration("death") +
		rand_double(SkeletonHalberd_consts::CORPSE_LIFETIME_MIN, SkeletonHalberd_consts::CORPSE_LIFETIME_MAX)
	);
}

bool m::enemy::SkeletonHalberd::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < SkeletonHalberd_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < SkeletonHalberd_consts::AGGRO_RANGE_Y;
}

bool m::enemy::SkeletonHalberd::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > SkeletonHalberd_consts::DEAGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) > SkeletonHalberd_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::SkeletonHalberd::aggroTransition() {
	this->_sprite->animation_play("move", true);
}

void m::enemy::SkeletonHalberd::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::SkeletonHalberd::update_when_aggroed(Milliseconds elapsedTime) {
	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto attackHitbox = dRect(
		this->position.x - (this->orientation == Orientation::RIGHT ? SkeletonHalberd_consts::ATTACK_RANGE_BACK : SkeletonHalberd_consts::ATTACK_RANGE_FRONT),
		this->position.y - SkeletonHalberd_consts::ATTACK_RANGE_UP,
		SkeletonHalberd_consts::ATTACK_RANGE_FRONT + SkeletonHalberd_consts::ATTACK_RANGE_BACK,
		SkeletonHalberd_consts::ATTACK_RANGE_UP + SkeletonHalberd_consts::ATTACK_RANGE_DOWN
	); // nasty geometry

	const auto hitboxOverlapArea = attackHitbox.collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > 12.) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				SkeletonHalberd_consts::MOVEMENT_FORCE_CHASE,
				SkeletonHalberd_consts::MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked() && hitboxOverlapArea > SkeletonHalberd_consts::HITBOX_OVERLAP_REQUIRED_TO_ATTACK && this->_sprite->animation_rushToEnd(3.)) {
			this->_sprite->animation_play("attack_windup");

			this->state_change(State::ATTACK_WINDUP);
			this->state_lock(this->_sprite->animation_duration("attack_windup"));
		}

		break;

	case State::ATTACK_WINDUP:
		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			if (hitboxOverlapArea > 0.) {
				this->target->health->applyDamage(SkeletonHalberd_consts::ATTACK_DAMAGE);
				this->target->solid->addImpulse_Horizontal(SkeletonHalberd_consts::ATTACK_KNOCKBACK_X * sign);
				this->target->solid->addImpulse_Up(SkeletonHalberd_consts::ATTACK_KNOCKBACK_Y);
			}

			this->_sprite->animation_play("attack_recover");

			this->state_change(State::ATTACK_RECOVER);
			this->state_lock(this->_sprite->animation_duration("attack_recover"));
		}
		break;

	case State::ATTACK_RECOVER:
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			this->_sprite->animation_play("move", true);

			this->state_change(State::CHASE);
		}
		break;

	default:
		break;
	}
}

void m::enemy::SkeletonHalberd::update_when_deaggroed(Milliseconds elapsedTime) {
	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::WANDER_STAND:
		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);
				this->_sprite->animation_play("move", true);

				this->state_change(State::WANDER_MOVE);
				this->state_lock(rand_double(SkeletonHalberd_consts::WANDER_MOVE_TIMER_MIN, SkeletonHalberd_consts::WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_lock(rand_double(SkeletonHalberd_consts::WANDER_STAND_TIMER_MIN, SkeletonHalberd_consts::WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	case State::WANDER_MOVE:
		this->solid->applyForceTillMaxSpeed_Horizontal(
			SkeletonHalberd_consts::MOVEMENT_FORCE_WANDER,
			SkeletonHalberd_consts::MOVEMENT_SPEED_WANDER,
			this->orientation
		);

		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);
				this->_sprite->animation_play("move", true);

				this->state_lock(rand_double(SkeletonHalberd_consts::WANDER_MOVE_TIMER_MIN, SkeletonHalberd_consts::WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::WANDER_STAND);
				this->state_lock(rand_double(SkeletonHalberd_consts::WANDER_STAND_TIMER_MIN, SkeletonHalberd_consts::WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	default:
		break;
	}
}

void m::enemy::SkeletonHalberd::deathTransition() {
	Enemy::deathTransition();

	this->solid->mass *= 20.; // so the pile of bones doesn't get pushed around like a feather
	this->_sprite->animation_play("death");
}



/* ### item_entity:: ### */

// # BrassRelic #
m::item_entity::BrassRelic::BrassRelic(const Vector2d &position) :
	ItemEntity(position)
{
	// Init members
	this->name = "brass_relic";

	// Init modules
	this->_init_sprite(true, "item_brass_relic");

	this->_init_solid(Vector2d(14., 10.));
}



// # Paper #
m::item_entity::Paper::Paper(const Vector2d &position) :
	ItemEntity(position)
{
	// Init members
	this->name = "paper";

	// Init modules
	this->_init_sprite(false, "item_paper");

	this->_init_solid(Vector2d(14., 14.));
}



/* ### destructible:: ### */

// # TNT #
m::destructible::TNT::TNT(const Vector2d &position) :
	Destructible(position)
{
	// Init members
	this->_init_delay(300.);

	/// Init modules
	/*this->_init_sprite(
		"tnt.png",
		{
			std::make_pair(make_srcRect(0, 0, 15, 15), 0)
		},
		{
			std::make_pair(make_srcRect(0, 15, 30, 30), 100),
			std::make_pair(make_srcRect(30, 15, 30, 30), 100),
			std::make_pair(make_srcRect(60, 15, 30, 30), 100)
		});*/

	this->_init_solid(Vector2d(15., 15.));

	this->_init_health(Faction::PROPS, 700);
}

void m::destructible::TNT::effect() {
	const dRect area(this->position, Vector2d(50., 50.), true);
	const Damage damage(this->health->faction, 400);
	Game::ACCESS->level.damageInArea(area, damage);
}