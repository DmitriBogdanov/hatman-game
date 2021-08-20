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
	{"enemy-skeleton_halberd", make_derived<m::enemy::SkeletonHalberd>},
	{"enemy-devourer", make_derived<m::enemy::Devourer>}
	/// new entities go there
};

std::unique_ptr<Entity> m::make_entity(const std::string &type, const std::string &name, const Vector2d &position) {
	return ENTITY_MAKERS.at(type + '-' + name)(position); // type and name are joined into a single string
}



/* ### enemy:: ### */

// # Sludge #
namespace Sludge_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(14., 13.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 120.;
	constexpr double FRICTION = 0.6;

	// Stats
	constexpr Faction FACTION = Faction::UNDEAD;
	constexpr uint MAX_HP = 1900;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 50;
	constexpr sint MAGIC_RES = 0;
	constexpr sint DOT_RES = 60;

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

	// Attack
	constexpr Milliseconds ATTACK_CD = 550.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 150., 0., 50., 0.);
	constexpr double ATTACK_KNOCKBACK_X = 200. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 150. * 100.;
	
	// Behaviour
	constexpr double AGGRO_RANGE_X = 200.;
	constexpr double AGGRO_RANGE_Y = 40.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 250.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(8.);

	// Death
	constexpr int PARTICLE_COUNT = 16;
	constexpr double PARTICLE_MAX_SPEED_X = 200.;
	constexpr double PARTICLE_MAX_SPEED_Y = 250.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);

}

m::enemy::Sludge::Sludge(const Vector2d &position) :
	Enemy(position)
{
	using namespace Sludge_consts;

	// Init modules
	this->_init_sprite("sludge", { DEFAULT_ANIMATION_NAME });

	this->_init_solid(
		HITBOX_SIZE,
		SOLID_FLAGS,
		MASS,
		FRICTION
	);

	this->_init_health(
		FACTION,
		MAX_HP,
		REGEN,
		PHYS_RES,
		MAGIC_RES,
		DOT_RES
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
		std::abs(creatureRelativePos.x) > Sludge_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Sludge_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Sludge::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Sludge_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > 10.) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;
		}

		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_CHASE,
			MOVEMENT_SPEED_CHASE,
			this->orientation
		);
		
		// Transition
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_tryChange(State::ATTACK);
		}

		break;

	case State::ATTACK:
		if (this->attack_cd.finished()) {
			this->target->health->applyDamage(ATTACK_DAMAGE);
			this->target->solid->addImpulse_Horizontal(ATTACK_KNOCKBACK_X * sign);
			this->target->solid->addImpulse_Up(ATTACK_KNOCKBACK_Y);

			this->attack_cd.start(ATTACK_CD);
		}

		// Transition
		if (hitboxOverlapArea < HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_tryChange(State::CHASE);
		}

		break;

	default:
		break;
	}
}

void m::enemy::Sludge::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace Sludge_consts;

	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::WANDER_STAND:
		// Transition
		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

				this->state_change(State::WANDER_MOVE);
				this->state_lock(rand_double(WANDER_MOVE_TIMER_MIN, WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	case State::WANDER_MOVE:
		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_WANDER,
			MOVEMENT_SPEED_WANDER,
			this->orientation
		);

		// Transition
		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

				this->state_lock(rand_double(WANDER_MOVE_TIMER_MIN, WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->state_change(State::WANDER_STAND);
				this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	default:
		break;
	}
}

void m::enemy::Sludge::deathTransition() {
	Enemy::deathTransition();

	using namespace Sludge_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # SkeletonHalberd #
namespace SkeletonHalberd_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(10., 24.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 140.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::UNDEAD;
	constexpr uint MAX_HP = 1300;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 20;
	constexpr sint MAGIC_RES = 20;
	constexpr sint DOT_RES = 20;

	
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

	// Attack
	constexpr Milliseconds ATTACK_CD = 2000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 350., 0., 0., 0.);
	constexpr double ATTACK_KNOCKBACK_X = 250. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	constexpr double ATTACK_RANGE_FRONT = 30.;
	constexpr double ATTACK_RANGE_BACK = 2.;
	constexpr double ATTACK_RANGE_UP = 20.;
	constexpr double ATTACK_RANGE_DOWN = 12.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(2.);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 280.;
	constexpr double AGGRO_RANGE_Y = 40.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 250.;
	
	// Death
	constexpr Milliseconds CORPSE_LIFETIME_MIN = sec_to_ms(1.);
	constexpr Milliseconds CORPSE_LIFETIME_MAX = sec_to_ms(2.);
}

m::enemy::SkeletonHalberd::SkeletonHalberd(const Vector2d &position) :
	Enemy(position)
{
	using namespace SkeletonHalberd_consts;

	// Init modules
	this->_init_sprite("skeleton_halberd", { DEFAULT_ANIMATION_NAME, "move", "attack_windup", "attack_recover", "death" });

	this->_init_solid(
		HITBOX_SIZE,
		SOLID_FLAGS,
		MASS,
		FRICTION
	);

	this->_init_health(
		FACTION,
		MAX_HP,
		REGEN,
		PHYS_RES,
		MAGIC_RES,
		DOT_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));

	this->_optinit_death_delay(
		this->_sprite->animation_duration("death") +
		rand_double(CORPSE_LIFETIME_MIN, CORPSE_LIFETIME_MAX)
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
		std::abs(creatureRelativePos.x) > SkeletonHalberd_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > SkeletonHalberd_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::SkeletonHalberd::aggroTransition() {
	this->_sprite->animation_play("move", true);
}

void m::enemy::SkeletonHalberd::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::SkeletonHalberd::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace SkeletonHalberd_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto attackHitbox = dRect(
		this->position.x - (this->orientation == Orientation::RIGHT ? ATTACK_RANGE_BACK : ATTACK_RANGE_FRONT),
		this->position.y - ATTACK_RANGE_UP,
		ATTACK_RANGE_FRONT + ATTACK_RANGE_BACK,
		ATTACK_RANGE_UP + ATTACK_RANGE_DOWN
	); // nasty geometry

	const auto hitboxOverlapArea = attackHitbox.collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > 12.) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_CHASE,
				MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked() && hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK && this->_sprite->animation_rushToEnd(3.)) {
			this->_sprite->animation_play("attack_windup");

			this->state_change(State::ATTACK_WINDUP);
			this->state_lock(this->_sprite->animation_duration("attack_windup"));
		}

		break;

	case State::ATTACK_WINDUP:
		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			if (hitboxOverlapArea > 0.) {
				this->target->health->applyDamage(ATTACK_DAMAGE);
				this->target->solid->addImpulse_Horizontal(ATTACK_KNOCKBACK_X * sign);
				this->target->solid->addImpulse_Up(ATTACK_KNOCKBACK_Y);
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
	using namespace SkeletonHalberd_consts;

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
				this->state_lock(rand_double(WANDER_MOVE_TIMER_MIN, WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
			}
		}
		break;

	case State::WANDER_MOVE:
		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_WANDER,
			MOVEMENT_SPEED_WANDER,
			this->orientation
		);

		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);
				this->_sprite->animation_play("move", true);

				this->state_lock(rand_double(WANDER_MOVE_TIMER_MIN, WANDER_MOVE_TIMER_MAX));
			}
			else {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::WANDER_STAND);
				this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
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



// # Devourer #
namespace Devourer_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(14., 28.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 190.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::UNDEAD;
	constexpr uint MAX_HP = 2100;
	constexpr sint REGEN = 60;
	constexpr sint PHYS_RES = 0;
	constexpr sint MAGIC_RES = 30;
	constexpr sint DOT_RES = 0;

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 100.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 900.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Attack
	constexpr Milliseconds ATTACK_CD = 1000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 100., 0., 0., 100.);
	constexpr double ATTACK_KNOCKBACK_X = 150. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(3.);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 150.;
	constexpr double AGGRO_RANGE_Y = 30.;
	constexpr double DEAGGRO_RANGE_X = 200.;
	constexpr double DEAGGRO_RANGE_Y = 40.;

	constexpr Milliseconds TIME_BEFORE_ATTACK = sec_to_ms(0.5); // time before Devourer attacks after not looking at him
	constexpr Milliseconds TIME_BEFORE_STOP = sec_to_ms(0.2); // time before Devourer stops after looking at him
	constexpr double IMMINENT_ATTACK_RANGE = 32.; // range in which Devourer attacks regardless

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	constexpr double ANIMATION_SPEEDUP = 2.;

	// Death
	constexpr int PARTICLE_COUNT = 20;
	constexpr double PARTICLE_MAX_SPEED_X = 250.;
	constexpr double PARTICLE_MAX_SPEED_Y = 250.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);
}

m::enemy::Devourer::Devourer(const Vector2d& position) :
	Enemy(position),
	time_target_was_looking_away(0.),
	time_target_was_looking_towards(0.)
{
	using namespace Devourer_consts;

	// Init modules
	this->_init_sprite("devourer", { DEFAULT_ANIMATION_NAME, "move" });

	this->_init_solid(
		HITBOX_SIZE,
		SOLID_FLAGS,
		MASS,
		FRICTION
	);

	this->_init_health(
		FACTION,
		MAX_HP,
		REGEN,
		PHYS_RES,
		MAGIC_RES,
		DOT_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::STAND);
	this->_init_default_deaggroed_state(State::STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::Devourer::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Devourer_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Devourer_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Devourer::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Devourer_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Devourer_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Devourer::aggroTransition() {
	this->state_lock();
}

void m::enemy::Devourer::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::Devourer::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Devourer_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);
	const auto targetLooksAway = targetPosSign * sign(this->target->orientation) > 0.;

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::STAND:
		if (targetLooksAway) this->time_target_was_looking_away += elapsedTime;
		else this->time_target_was_looking_away = 0.;

		// Look at the target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->time_target_was_looking_away > TIME_BEFORE_ATTACK || std::abs(this->target_relative_pos.x) < IMMINENT_ATTACK_RANGE)
			this->state_unlock();

		if (this->state_isUnlocked() && this->_sprite->animation_rushToEnd(ANIMATION_SPEEDUP)) {
			this->_sprite->animation_play("move", true);
			this->state_change(State::CHASE);
			this->state_lock();
		}

		break;

	case State::CHASE:
		if (!targetLooksAway) this->time_target_was_looking_towards += elapsedTime;
		else this->time_target_was_looking_towards = 0.;

		// Run towards target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE) {
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_CHASE,
				MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Attack if target in range
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK && this->attack_cd.finished()) {
			this->target->health->applyDamage(ATTACK_DAMAGE);
			this->target->solid->addImpulse_Horizontal(ATTACK_KNOCKBACK_X * targetPosSign);
			this->target->solid->addImpulse_Up(ATTACK_KNOCKBACK_Y);

			this->attack_cd.start(ATTACK_CD);
		}

		// Transition
		if (this->time_target_was_looking_towards > TIME_BEFORE_STOP && std::abs(this->target_relative_pos.x) > IMMINENT_ATTACK_RANGE)
			this->state_unlock();

		if (this->state_isUnlocked() && this->_sprite->animation_rushToEnd(ANIMATION_SPEEDUP)) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::STAND);
			this->state_lock();
		}

		break;

	default:
		break;
	}
}

void m::enemy::Devourer::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::Devourer::deathTransition() {
	Enemy::deathTransition();

	using namespace Devourer_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
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
	///Game::ACCESS->level->damageInArea(area, damage);
}