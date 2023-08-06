#include "entity_unique_m.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)

#include "game.h" // access to game state
#include "controls.h" // access to control keys
#include "entity_unique_s.h" // particles and projectiles
#include "ct_math.hpp" // for compile-time math
#include "globalconsts.hpp" // physical consts
#include "debug_tools.hpp" /// TEMP



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
	// enemies
	{"enemy-sludge", make_derived<m::enemy::Sludge>},
	{"enemy-worm", make_derived<m::enemy::Worm>},
	{"enemy-golem", make_derived<m::enemy::Golem>},
	{"enemy-skeleton_halberd", make_derived<m::enemy::SkeletonHalberd>},
	{"enemy-pygmy_warrior", make_derived<m::enemy::PygmyWarrior>},
	{"enemy-devourer", make_derived<m::enemy::Devourer>},
	{"enemy-spirit_bomber", make_derived<m::enemy::SpiritBomber>},
	{"enemy-cultist_mage", make_derived<m::enemy::CultistMage>},
	{"enemy-hellhound", make_derived<m::enemy::Hellhound>},
	{"enemy-necromancer", make_derived<m::enemy::Necromancer>},
	{"enemy-tentacle", make_derived<m::enemy::Tentacle>},
	{"enemy-boss_mage_1", make_derived<m::enemy::BossMage1>},
	{"enemy-boss_mage_2", make_derived<m::enemy::BossMage2>},
	{"enemy-boss_mage_3", make_derived<m::enemy::BossMage3>},
	// items
	{"item-eldritch_battery", make_derived<m::item_entity::EldritchBattery>},
	{"item-power_shard", make_derived<m::item_entity::PowerShard>},
	{"item-spider_signet", make_derived<m::item_entity::SpiderSignet>},
	{"item-bone_mask", make_derived<m::item_entity::BoneMask>},
	{"item-magic_negator", make_derived<m::item_entity::MagicNegator>},
	{"item-twin_souls", make_derived<m::item_entity::TwinSouls>},
	{"item-watching_eye", make_derived<m::item_entity::WatchingEye>},
	// destructibles
	{"destructible-orb_of_betrayal", make_derived<m::destructible::OrbOfBetrayal>}

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
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 1900;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 50;
	constexpr sint MAGIC_RES = 0;
	constexpr sint CHAOS_RES = 60;

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

	constexpr double CHASE_UNTIL_RANGE = 9.;

	// Attack
	constexpr Milliseconds ATTACK_CD = 500.;
	constexpr auto ATTACK_DAMAGE = Damage(FACTION, 100., 0., 100., 30.);
	constexpr double ATTACK_KNOCKBACK_X = 200. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 150. * 100.;
	
	// Behaviour
	constexpr double AGGRO_RANGE_X = 150.;
	constexpr double AGGRO_RANGE_Y = 30.;
	constexpr double DEAGGRO_RANGE_X = 200.;
	constexpr double DEAGGRO_RANGE_Y = 80.;

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
	this->_init_sprite("[enemy]{sludge}", { DEFAULT_ANIMATION_NAME });

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
		CHAOS_RES
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

	// NOTE: Aggro'ed Sludge has no sate locks which is why there is no
	// state_isUnlocked() condition anywhere

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;
		}

		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_CHASE,
			MOVEMENT_SPEED_CHASE,
			this->orientation
		);
		
		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Cannot pursue without falling => await
		else if (!this->_has_ground_in_front()) {
			this->state_change(State::AWAIT);
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Enemy can be chased witout falling => chase
		if (this->_has_ground_in_front()) {
			this->state_change(State::CHASE);
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
			this->state_change(State::CHASE);
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
		if (!this->_has_ground_in_front()) {
			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
			// not falling off is so important that we ignore state lock

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



// # Worm #
namespace Worm_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(16., 8.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 60.;
	constexpr double FRICTION = 0.8;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 500;
	constexpr sint REGEN = 50;
	constexpr sint PHYS_RES = 10;
	constexpr sint MAGIC_RES = 10;
	constexpr sint CHAOS_RES = 60;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 60.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 180.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(0.2);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(0.5);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.8);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(2.);

	constexpr double WANDER_MOVE_CHANCE = 0.75;

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 60.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 180.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	constexpr double CHASE_UNTIL_RANGE = 7.;

	// Attack
	constexpr Milliseconds ATTACK_CD = sec_to_ms(0.3);
	const auto ATTACK_DAMAGE = Damage(FACTION, 30., 0., 30., 20.);
	constexpr double ATTACK_KNOCKBACK_X = 50. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 150.;
	constexpr double AGGRO_RANGE_Y = 30.;
	constexpr double DEAGGRO_RANGE_X = 200.;
	constexpr double DEAGGRO_RANGE_Y = 90.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(4.);

	// Death
	constexpr int PARTICLE_COUNT = 8;
	constexpr double PARTICLE_MAX_SPEED_X = 200.;
	constexpr double PARTICLE_MAX_SPEED_Y = 250.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);

}

m::enemy::Worm::Worm(const Vector2d &position) :
	Enemy(position)
{
	using namespace Worm_consts;

	// Init modules
	this->_init_sprite("[enemy]{worm}", { DEFAULT_ANIMATION_NAME });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_MOVE);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::Worm::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Worm_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Worm_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Worm::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Worm_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Worm_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Worm::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Worm_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	// NOTE: Aggro'ed Worm has no sate locks which is why there is no
	// state_isUnlocked() condition anywhere

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;
		}

		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_CHASE,
			MOVEMENT_SPEED_CHASE,
			this->orientation
		);

		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Cannot pursue without falling => await
		else if (!this->_has_ground_in_front()) {
			this->state_change(State::AWAIT);
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Enemy can be chased witout falling => chase
		if (this->_has_ground_in_front()) {
			this->state_change(State::CHASE);
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
			this->state_change(State::CHASE);
		}

		break;

	default:
		break;
	}
}

void m::enemy::Worm::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace Worm_consts;

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
		if (!this->_has_ground_in_front()) {
			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
		// not falling off is so important that we ignore state lock

		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_double() < WANDER_MOVE_CHANCE;

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

void m::enemy::Worm::deathTransition() {
	Enemy::deathTransition();

	using namespace Worm_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # Golem #
namespace Golem_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(22., 28.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 300.;
	constexpr double FRICTION = 0.8;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 3500;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 40;
	constexpr sint MAGIC_RES = 0;
	constexpr sint CHAOS_RES = 0;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 30.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 90.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(3.);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(4.);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(1.);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(2.);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 40.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 90.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	constexpr double CHASE_UNTIL_RANGE = 8.;

	// Attack
	constexpr Milliseconds ATTACK_CD = 1000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 300., 0., 0., 50.);
	constexpr double ATTACK_KNOCKBACK_X = 250. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 150. * 100.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 150.;
	constexpr double AGGRO_RANGE_Y = 20.;
	constexpr double DEAGGRO_RANGE_X = 250.;
	constexpr double DEAGGRO_RANGE_Y = 60.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(8.);

	// Death
	constexpr int PARTICLE_COUNT = 40;
	constexpr double PARTICLE_MAX_SPEED_X = 150.;
	constexpr double PARTICLE_MAX_SPEED_Y = 200.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(3.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(7.);

}

m::enemy::Golem::Golem(const Vector2d &position) :
	Enemy(position)
{
	using namespace Golem_consts;

	// Init modules
	this->_init_sprite("[enemy]{golem}", { DEFAULT_ANIMATION_NAME, "move" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::Golem::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Golem_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Golem_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Golem::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Golem_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Golem_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Golem::aggroTransition() {
	this->_sprite->animation_play("move", true);
}

void m::enemy::Golem::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::Golem::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Golem_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	// NOTE: Aggro'ed Golem has no sate locks which is why there is no
	// state_isUnlocked() condition anywhere

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;
		}

		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_CHASE,
			MOVEMENT_SPEED_CHASE,
			this->orientation
		);

		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Cannot pursue without falling => await
		else if (!this->_has_ground_in_front()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::AWAIT);
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		// Target in range => attack
		if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->state_change(State::ATTACK);
		}
		// Enemy can be chased witout falling => chase
		if (this->_has_ground_in_front()) {
			this->_sprite->animation_play("move", true);
			this->state_change(State::CHASE);
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
			this->_sprite->animation_play("move", true);
			this->state_change(State::CHASE);
		}

		break;

	default:
		break;
	}
}

void m::enemy::Golem::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace Golem_consts;

	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::WANDER_STAND:
		// Transition
		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

				this->_sprite->animation_play("move", true);
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
		if (!this->_has_ground_in_front()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
		// not falling off is so important that we ignore state lock

		if (this->state_isUnlocked()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);

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

void m::enemy::Golem::deathTransition() {
	Enemy::deathTransition();

	using namespace Golem_consts;

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
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 2100;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 20;
	constexpr sint MAGIC_RES = 20;
	constexpr sint CHAOS_RES = 20;

	
	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 40.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 60.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(0.5);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(2.0);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.3);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(0.9);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 80.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 300.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	constexpr double CHASE_UNTIL_RANGE = 12.;

	// Attack
	constexpr Milliseconds ATTACK_CD = 2000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 200., 0., 0., 150.);
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
	constexpr double DEAGGRO_RANGE_Y = 100.;
	
	// Death
	constexpr Milliseconds CORPSE_LIFETIME_MIN = sec_to_ms(4.);
	constexpr Milliseconds CORPSE_LIFETIME_MAX = sec_to_ms(6.);
}

m::enemy::SkeletonHalberd::SkeletonHalberd(const Vector2d &position) :
	Enemy(position)
{
	using namespace SkeletonHalberd_consts;

	// Init modules
	this->_init_sprite("[enemy]{skeleton_halberd}", { DEFAULT_ANIMATION_NAME, "move", "attack_windup", "attack_recover", "death" });

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
		CHAOS_RES
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
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_CHASE,
				MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be attacked => attack
			if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK && this->_sprite->animation_rushToEnd(3.)) {
				this->_sprite->animation_play("attack_windup");

				this->state_change(State::ATTACK_WINDUP);
				this->state_lock(this->_sprite->animation_duration("attack_windup"));
			}
			// Cannot pursue without falling
			// OR
			// Enemy close horizontally but unreachable
			// => await
			else if (!this->_has_ground_in_front() || std::abs(this->target_relative_pos.x) < CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::AWAIT);
			}
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be chased without falling
			// AND
			// Enemy not close horizontally
			// => chase
			if (this->_has_ground_in_front() && std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play("move", true);

				this->state_change(State::CHASE);
			}
			// Enemy reachable by attack => attack
			else if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
				this->_sprite->animation_play("attack_windup");

				this->state_change(State::ATTACK_WINDUP);
				this->state_lock(this->_sprite->animation_duration("attack_windup"));
			}
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
		if (!this->_has_ground_in_front()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
			// not falling off is so important that we ignore state lock

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



// # PygmyWarrior #
namespace PygmyWarrior_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(16., 16.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 90.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 1400;
	constexpr sint REGEN = 40;
	constexpr sint PHYS_RES = 20;
	constexpr sint MAGIC_RES = 30;
	constexpr sint CHAOS_RES = 20;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 100.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 300.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(0.5);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(1.5);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.4);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(0.9);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 100.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 300.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	constexpr double CHASE_UNTIL_RANGE = 10.;

	// Attack
	///constexpr Milliseconds ATTACK_CD = 1000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 250., 0., 0., 50.);
	constexpr double ATTACK_KNOCKBACK_X = 150. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	constexpr double ATTACK_RANGE_FRONT = 22.;
	constexpr double ATTACK_RANGE_BACK = 6.;
	constexpr double ATTACK_RANGE_UP = 10.;
	constexpr double ATTACK_RANGE_DOWN = 10.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(2.);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 200.;
	constexpr double AGGRO_RANGE_Y = 20.;
	constexpr double DEAGGRO_RANGE_X = 260.;
	constexpr double DEAGGRO_RANGE_Y = 60.;
}

m::enemy::PygmyWarrior::PygmyWarrior(const Vector2d &position) :
	Enemy(position)
{
	using namespace PygmyWarrior_consts;

	// Init modules
	this->_init_sprite("[enemy]{pygmy_warrior}", { DEFAULT_ANIMATION_NAME, "move", "attack_windup", "attack_recover", "death" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));

	this->_optinit_death_delay(
		this->_sprite->animation_duration("death")
	);
}

bool m::enemy::PygmyWarrior::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < PygmyWarrior_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < PygmyWarrior_consts::AGGRO_RANGE_Y;
}

bool m::enemy::PygmyWarrior::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > PygmyWarrior_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > PygmyWarrior_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::PygmyWarrior::aggroTransition() {
	this->_sprite->animation_play("move", true);
}

void m::enemy::PygmyWarrior::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::PygmyWarrior::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace PygmyWarrior_consts;

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
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_CHASE,
				MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be attacked => attack
			if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK && this->_sprite->animation_rushToEnd(3.)) {
				this->_sprite->animation_play("attack_windup");

				this->state_change(State::ATTACK_WINDUP);
				this->state_lock(this->_sprite->animation_duration("attack_windup"));
			}
			// Cannot pursue without falling
			// OR
			// Enemy close horizontally but unreachable
			// => await
			else if (!this->_has_ground_in_front() || std::abs(this->target_relative_pos.x) < CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::AWAIT);
			}
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be chased without falling
			// AND
			// Enemy not close horizontally
			// => chase
			if (this->_has_ground_in_front() && std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play("move", true);

				this->state_change(State::CHASE);
			}
			// Enemy reachable by attack => attack
			else if (hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
				this->_sprite->animation_play("attack_windup");

				this->state_change(State::ATTACK_WINDUP);
				this->state_lock(this->_sprite->animation_duration("attack_windup"));
			}
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

void m::enemy::PygmyWarrior::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace PygmyWarrior_consts;

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
		if (!this->_has_ground_in_front()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
		// not falling off is so important that we ignore state lock

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

void m::enemy::PygmyWarrior::deathTransition() {
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
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 2100;
	constexpr sint REGEN = 60;
	constexpr sint PHYS_RES = 0;
	constexpr sint MAGIC_RES = 30;
	constexpr sint CHAOS_RES = 0;

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 100.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 900.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Attack
	constexpr Milliseconds ATTACK_CD = 400.;
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

	///constexpr double IMMINENT_ATTACK_RANGE = 32.; // range in which Devourer attacks regardless

	constexpr double IMMINENT_ATTACK_HP_PERCANTAGE = 0.99; // when hp below that Devourer attacks regardless

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
	this->_init_sprite("[enemy]{devourer}", { DEFAULT_ANIMATION_NAME, "move" });

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
		CHAOS_RES
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
	const bool targetLooksAway = targetPosSign * sign(this->target->orientation) > 0.;

	const bool retaliation = this->health->percentage() < IMMINENT_ATTACK_HP_PERCANTAGE;

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::STAND:
		if (targetLooksAway) this->time_target_was_looking_away += elapsedTime;
		else this->time_target_was_looking_away = 0.;

		// Look at the target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->time_target_was_looking_away > TIME_BEFORE_ATTACK || retaliation)
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
		if (this->time_target_was_looking_towards > TIME_BEFORE_STOP && !retaliation)
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



// # SpiritBomber #
namespace SpiritBomber_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(12., 16.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 80.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 900;
	constexpr sint REGEN = 60;
	constexpr sint PHYS_RES = 0;
	constexpr sint MAGIC_RES = 40;
	constexpr sint CHAOS_RES = 0;

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 100.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 900.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Attack
	constexpr Milliseconds ATTACK_CD = sec_to_ms(1.7);
	const auto ATTACK_DAMAGE = Damage(FACTION, 0., 200., 0., 100.);
	constexpr double ATTACK_KNOCKBACK = 110. * 100.;
	constexpr double ATTACK_PROJECTILE_SPEED = 200.;
	constexpr Vector2d ATTACK_AOE = Vector2d(20, 20);

	constexpr double PROJECTILE_SPAWN_ALIGNMENT_X = -2.;
	constexpr double PROJECTILE_SPAWN_ALIGNMENT_Y = -4.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 90.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 120.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	constexpr double ANIMATION_SPEEDUP = 2.;

	// Death
	constexpr int PARTICLE_COUNT = 10;
	constexpr double PARTICLE_MAX_SPEED_X = 150.;
	constexpr double PARTICLE_MAX_SPEED_Y = 150.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);
}

m::enemy::SpiritBomber::SpiritBomber(const Vector2d& position) :
	Enemy(position)
{
	using namespace SpiritBomber_consts;

	// Init modules
	this->_init_sprite("[enemy]{spirit_bomber}", { DEFAULT_ANIMATION_NAME, "attack_windup", "attack_recover" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::STAND);
	this->_init_default_deaggroed_state(State::STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::SpiritBomber::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < SpiritBomber_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < SpiritBomber_consts::AGGRO_RANGE_Y;
}

bool m::enemy::SpiritBomber::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > SpiritBomber_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > SpiritBomber_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::SpiritBomber::aggroTransition() {
	this->state_lock();
}

void m::enemy::SpiritBomber::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::SpiritBomber::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace SpiritBomber_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::STAND:
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->attack_cd.finished())
			this->state_unlock();
		
		if (this->state_isUnlocked() && this->_sprite->animation_rushToEnd(ANIMATION_SPEEDUP)) {
			this->_sprite->animation_play("attack_windup");
			this->state_change(State::ATTACK_WINDUP);
			this->state_lock();
		}

		break;

	case State::ATTACK_WINDUP:
		// Transition
		if (this->_sprite->animation_finished())
			this->state_unlock();
		
		if (this->state_isUnlocked()) {
			// Spawn projectile aimed at the target
			Game::ACCESS->level->spawn(std::make_unique<s::projectile::SpiritBomb>(
				this->position + Vector2d(PROJECTILE_SPAWN_ALIGNMENT_X * sign(this->orientation), PROJECTILE_SPAWN_ALIGNMENT_Y),
				this->target_relative_pos.normalized() * ATTACK_PROJECTILE_SPEED,
				ATTACK_DAMAGE,
				ATTACK_KNOCKBACK,
				ATTACK_AOE)
			);
			this->attack_cd.start(ATTACK_CD);

			this->_sprite->animation_play("attack_recover");
			this->state_change(State::ATTACK_RECOVER);
			this->state_lock();
		}

		break;

	case State::ATTACK_RECOVER:
		// Transitions
		if (this->_sprite->animation_finished())
			this->state_unlock();

		if (this->state_isUnlocked()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::STAND);
			this->state_lock();
		}

	default:
		break;
	}
}

void m::enemy::SpiritBomber::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::SpiritBomber::deathTransition() {
	Enemy::deathTransition();

	using namespace SpiritBomber_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # CultistMage #
namespace CultistMage_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(12., 32.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 120.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 1100;
	constexpr sint REGEN = 60;
	constexpr sint PHYS_RES = 0;
	constexpr sint MAGIC_RES = 30;
	constexpr sint CHAOS_RES = 70;

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 100.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 900.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	// Attack
	constexpr Milliseconds ATTACK_CD = sec_to_ms(2.5);
	const auto ATTACK_DAMAGE = Damage(FACTION, 0., 300., 100., 100.);
	constexpr double ATTACK_KNOCKBACK = 120. * 100.;
	constexpr double ATTACK_PROJECTILE_SPEED = 100.;
	constexpr Vector2d ATTACK_AOE = Vector2d(20, 20);

	constexpr double PROJECTILE_SPAWN_ALIGNMENT_Y = 12.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 100.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 120.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	constexpr double ANIMATION_SPEEDUP = 2.;

	// Death
	constexpr int PARTICLE_COUNT = 10;
	constexpr double PARTICLE_MAX_SPEED_X = 150.;
	constexpr double PARTICLE_MAX_SPEED_Y = 150.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);
}

m::enemy::CultistMage::CultistMage(const Vector2d& position) :
	Enemy(position)
{
	using namespace CultistMage_consts;

	// Init modules
	this->_init_sprite("[enemy]{cultist_mage}", { DEFAULT_ANIMATION_NAME, "attack_windup", "attack_recover" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::STAND);
	this->_init_default_deaggroed_state(State::STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::CultistMage::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < CultistMage_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < CultistMage_consts::AGGRO_RANGE_Y;
}

bool m::enemy::CultistMage::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > CultistMage_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > CultistMage_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::CultistMage::aggroTransition() {
	this->state_lock();
}

void m::enemy::CultistMage::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::CultistMage::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace CultistMage_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::STAND:
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->attack_cd.finished())
			this->state_unlock();

		if (this->state_isUnlocked() && this->_sprite->animation_rushToEnd(ANIMATION_SPEEDUP)) {
			this->_sprite->animation_play("attack_windup");
			this->state_change(State::ATTACK_WINDUP);
			this->state_lock();
		}

		break;

	case State::ATTACK_WINDUP:
		// Transition
		if (this->_sprite->animation_finished())
			this->state_unlock();

		if (this->state_isUnlocked()) {
			// Spawn projectile flying upwards below the target
			{
				const double target_x = this->position.x + this->target_relative_pos.x;
				const double camera_bottom = Graphics::READ->camera->get_FOV_rect().getBottom();

				Game::ACCESS->level->spawn(std::make_unique<s::projectile::Fireball>(
					Vector2d(target_x, camera_bottom + PROJECTILE_SPAWN_ALIGNMENT_Y),
					Vector2d(0., -ATTACK_PROJECTILE_SPEED),
					ATTACK_DAMAGE,
					ATTACK_KNOCKBACK,
					ATTACK_AOE)
				);
			}
			
			this->attack_cd.start(ATTACK_CD);

			this->_sprite->animation_play("attack_recover");
			this->state_change(State::ATTACK_RECOVER);
			this->state_lock();
		}

		break;

	case State::ATTACK_RECOVER:
		// Transitions
		if (this->_sprite->animation_finished())
			this->state_unlock();

		if (this->state_isUnlocked()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::STAND);
			this->state_lock();
		}

	default:
		break;
	}
}

void m::enemy::CultistMage::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::CultistMage::deathTransition() {
	Enemy::deathTransition();

	using namespace CultistMage_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # Hellhound #
namespace Hellhound_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(32, 32.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 190.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 4900;
	constexpr sint REGEN = 50;
	constexpr sint PHYS_RES = 30;
	constexpr sint MAGIC_RES = 30;
	constexpr sint CHAOS_RES = 40;

	// Wander
	constexpr double MOVEMENT_SPEED_WANDER = 70.;
	constexpr double MOVEMENT_ACCELERATION_WANDER = 300.;
	constexpr double MOVEMENT_FORCE_WANDER = MASS * MOVEMENT_ACCELERATION_WANDER;

	constexpr Milliseconds WANDER_STAND_TIMER_MIN = sec_to_ms(0.5);
	constexpr Milliseconds WANDER_STAND_TIMER_MAX = sec_to_ms(1.5);
	constexpr Milliseconds WANDER_MOVE_TIMER_MIN = sec_to_ms(0.4);
	constexpr Milliseconds WANDER_MOVE_TIMER_MAX = sec_to_ms(0.9);

	// Chase
	constexpr double MOVEMENT_SPEED_CHASE = 150.;
	constexpr double MOVEMENT_ACCELERATION_CHASE = 500.;
	constexpr double MOVEMENT_FORCE_CHASE = MASS * MOVEMENT_ACCELERATION_CHASE;

	constexpr double JUMP_IMPULSE_X = 400 * MASS;
	constexpr double JUMP_IMPULSE_Y = 200 * MASS;

	constexpr double CHASE_UNTIL_RANGE = 30.;

	// Flee
	constexpr double MOVEMENT_SPEED_FLEE = 150.;
	constexpr double MOVEMENT_ACCELERATION_FLEE = 500.;
	constexpr double MOVEMENT_FORCE_FLEE = MASS * MOVEMENT_ACCELERATION_FLEE;

	constexpr double FLEE_UNTIL_RANGE = 165.;

	constexpr Milliseconds MAX_FLEE_TIME = sec_to_ms(4);

	// Jump
	constexpr double MOVEMENT_SPEED_JUMP = 250.;
	constexpr double MOVEMENT_ACCELERATION_JUMP = 1500.;
	constexpr double MOVEMENT_FORCE_JUMP = MASS * MOVEMENT_ACCELERATION_JUMP;

	// Attack
	constexpr Milliseconds ATTACK_CD = 1000.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 100., 100., 200., 0.);
	constexpr double ATTACK_KNOCKBACK_X = 150. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 50. * 100.;

	constexpr double ATTACK_RANGE_FRONT = 90.;
	constexpr double ATTACK_RANGE_BACK = 4.;
	constexpr double ATTACK_RANGE_UP = 10.;
	constexpr double ATTACK_RANGE_DOWN = 10.;

	constexpr double TRIGGER_HITBOX_OVERLAP_REQUIRED = ct_sqr(4.);
	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(4.);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 200.;
	constexpr double AGGRO_RANGE_Y = 20.;
	constexpr double DEAGGRO_RANGE_X = 260.;
	constexpr double DEAGGRO_RANGE_Y = 60.;

	// Death
	constexpr int PARTICLE_COUNT = 50;
	constexpr double PARTICLE_MAX_SPEED_X = 200.;
	constexpr double PARTICLE_MAX_SPEED_Y = 200.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(3.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(11.);
}

m::enemy::Hellhound::Hellhound(const Vector2d &position) :
	Enemy(position)
{
	using namespace Hellhound_consts;

	// Init modules
	this->_init_sprite("[enemy]{hellhound}", { DEFAULT_ANIMATION_NAME, "walk", "run", "jump" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::CHASE);
	this->_init_default_deaggroed_state(State::WANDER_STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));
}

bool m::enemy::Hellhound::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Hellhound_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Hellhound_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Hellhound::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Hellhound_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Hellhound_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Hellhound::aggroTransition() {
	this->_sprite->animation_play("run", true);
}

void m::enemy::Hellhound::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::Hellhound::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Hellhound_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto attackTriggerHitbox = dRect(
		this->position.x - (this->orientation == Orientation::RIGHT ? ATTACK_RANGE_BACK : ATTACK_RANGE_FRONT),
		this->position.y - ATTACK_RANGE_UP,
		ATTACK_RANGE_FRONT + ATTACK_RANGE_BACK,
		ATTACK_RANGE_UP + ATTACK_RANGE_DOWN
	); // nasty geometry

	const auto attackTriggerHitboxOverlapArea = attackTriggerHitbox.collideWithRect(this->target->solid->getHitbox()).overlap_area;

	const auto entityHitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	switch (currentState) {
	case State::CHASE:
		if (std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
			this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_CHASE,
				MOVEMENT_SPEED_CHASE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be attacked => attack
			if (attackTriggerHitboxOverlapArea > TRIGGER_HITBOX_OVERLAP_REQUIRED) {
				/*this->solid->speed.x = 0.;
				this->solid->addImpulse_Horizontal(JUMP_IMPULSE_X * sign);
				this->solid->addImpulse_Up(JUMP_IMPULSE_Y);*/

				this->_sprite->animation_play("jump");

				this->state_change(State::JUMP_ATTACK);
			}
			// Cannot pursue without falling
			// OR
			// Enemy close horizontally but unreachable
			// => await
			else if (!this->_has_ground_in_front() || std::abs(this->target_relative_pos.x) < CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::AWAIT);
			}
		}

		break;

	case State::FLEE:
		if (std::abs(this->target_relative_pos.x) < FLEE_UNTIL_RANGE) {
			this->orientation = -sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

			this->solid->applyForceTillMaxSpeed_Horizontal(
				MOVEMENT_FORCE_FLEE,
				MOVEMENT_SPEED_FLEE,
				this->orientation
			);
		}

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy far enought
			// OR
			// Was fleeing for too long (stuck)
			// => switch back to chase
			if (std::abs(this->target_relative_pos.x) >= FLEE_UNTIL_RANGE || this->flee_timer.finished()) {
				this->state_change(State::CHASE);
			}
			// Cannot flee without falling
			// OR
			// Enemy far horizontally but unreachable
			// => await
			else if (!this->_has_ground_in_front()) {
				this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

				this->state_change(State::AWAIT);
			}
		}

		break;

	case State::AWAIT:
		this->orientation = sign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->state_isUnlocked()) {
			// Enemy can be chased without falling
			// AND
			// Enemy not close horizontally
			// => chase
			if (this->_has_ground_in_front() && std::abs(this->target_relative_pos.x) > CHASE_UNTIL_RANGE) {
				this->_sprite->animation_play("run", true);

				this->state_change(State::CHASE);
			}
			// Enemy reachable by attack => attack
			else if (attackTriggerHitboxOverlapArea > TRIGGER_HITBOX_OVERLAP_REQUIRED) {
				/*this->solid->speed.x = 0.;
				this->solid->addImpulse_Horizontal(JUMP_IMPULSE_X * sign);
				this->solid->addImpulse_Up(JUMP_IMPULSE_Y);*/

				this->_sprite->animation_play("jump");

				this->state_change(State::JUMP_ATTACK);
			}
		}
		break;

	case State::JUMP_ATTACK:
		this->solid->applyForceTillMaxSpeed_Horizontal(
			MOVEMENT_FORCE_JUMP,
			MOVEMENT_SPEED_JUMP,
			this->orientation
		);

		if (this->attack_cd.finished() && entityHitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->target->health->applyDamage(ATTACK_DAMAGE);
			this->target->solid->addImpulse_Horizontal(ATTACK_KNOCKBACK_X * sign);
			this->target->solid->addImpulse_Up(ATTACK_KNOCKBACK_Y);

			this->attack_cd.start(ATTACK_CD);
		}

		// Transition
		if (this->_sprite->animation_awaitEnd()) {
			this->_sprite->animation_play("run", true);

			this->state_change(State::FLEE);
			this->flee_timer.start(MAX_FLEE_TIME);
		}
		break;

	default:
		break;
	}
}

void m::enemy::Hellhound::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace Hellhound_consts;

	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::WANDER_STAND:
		// Transition
		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);
				this->_sprite->animation_play("walk", true);

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
		if (!this->_has_ground_in_front()) {
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

			this->state_change(State::WANDER_STAND);
			this->state_lock(rand_double(WANDER_STAND_TIMER_MIN, WANDER_STAND_TIMER_MAX));
		}
		// not falling off is so important that we ignore state lock

		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			const auto nextPhaseIsMove = rand_bool();

			if (nextPhaseIsMove) {
				this->orientation = invert(this->orientation);
				this->_sprite->animation_play("walk", true);

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

void m::enemy::Hellhound::deathTransition() {
	Enemy::deathTransition();

	using namespace Hellhound_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # Necromancer #
namespace Necromancer_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(29., 43.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 180.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 3900;
	constexpr sint REGEN = 60;
	constexpr sint PHYS_RES = 20;
	constexpr sint MAGIC_RES = 35;
	constexpr sint CHAOS_RES = 35;

	// Attack
	constexpr Milliseconds ATTACK_CD = sec_to_ms(0.5);

	constexpr double SUMMON_CHANCE_SLUDGE = 0.1;
	constexpr double SUMMON_CHANCE_WORM = 1 - SUMMON_CHANCE_SLUDGE;

	constexpr double SUMMON_RANGE_X_MIN = 20;
	constexpr double SUMMON_RANGE_X_MAX = 40;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 20.;
	constexpr double DEAGGRO_RANGE_X = 300.;
	constexpr double DEAGGRO_RANGE_Y = 30.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	// Death
	constexpr Milliseconds CORPSE_LIFETIME_MIN = sec_to_ms(4.);
	constexpr Milliseconds CORPSE_LIFETIME_MAX = sec_to_ms(6.);
}

m::enemy::Necromancer::Necromancer(const Vector2d& position) :
	Enemy(position)
{
	using namespace Necromancer_consts;

	// Init modules
	this->_init_sprite("[enemy]{necromancer}", { DEFAULT_ANIMATION_NAME, "death" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::STAND);
	this->_init_default_deaggroed_state(State::STAND);

	const double HITBOX_GAP = 3.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));

	this->_optinit_death_delay(
		this->_sprite->animation_duration("death") +
		rand_double(CORPSE_LIFETIME_MIN, CORPSE_LIFETIME_MAX)
	);
}

bool m::enemy::Necromancer::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < Necromancer_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Necromancer_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Necromancer::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Necromancer_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Necromancer_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Necromancer::aggroTransition() {
	this->state_lock();
}

void m::enemy::Necromancer::deaggroTransition() {
	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::Necromancer::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Necromancer_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::STAND:
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Transition
		if (this->attack_cd.finished())
			this->state_unlock();

		if (this->state_isUnlocked() && this->_sprite->animation_awaitEnd()) {
			this->state_change(State::ATTACK);
		}

		break;

	case State::ATTACK: {

		// Create sludge or worm entity
		std::unique_ptr<Entity> spawned_creature;

		if (rand_double() < SUMMON_CHANCE_SLUDGE)
			spawned_creature = std::make_unique<m::enemy::Sludge>(this->position);
		else
			spawned_creature = std::make_unique<m::enemy::Worm>(this->position);
			// ternary can't be used here due to different argument types	

		// Select random spawn position (X) nearby
		const double spawn_x = this->position.x + rand_choise({ -1., 1. }) * rand_double(SUMMON_RANGE_X_MIN, SUMMON_RANGE_X_MAX);
		
		// Adjust spawn position (y) with respect to hitbox size
		const double ground_level = this->solid->getHitbox().getBottom();
		const double spawn_y = ground_level - 0.5 * spawned_creature->solid->getHitbox().getSizeY();

		// Spawn the creature
		spawned_creature->position.x = spawn_x;
		spawned_creature->position.y = spawn_y;
		Game::ACCESS->level->spawn(std::move(spawned_creature));

		// Transition
		// (happens instantly, no conditions involved)
		this->attack_cd.start(ATTACK_CD);
		this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
		this->state_change(State::STAND);
		this->state_lock();

		break;
	}
	default:
		break;
	}
}

void m::enemy::Necromancer::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::Necromancer::deathTransition() {
	Enemy::deathTransition();

	using namespace Necromancer_consts;

	this->solid->mass *= 20.; // so the pile of bones doesn't get pushed around like a feather
	this->_sprite->animation_play("death");
}



// # Tentacle #
namespace Tentacle_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(20., 70.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 1e3;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 200;
	constexpr sint REGEN = 50;
	constexpr sint PHYS_RES = 30;
	constexpr sint MAGIC_RES = 10;
	constexpr sint CHAOS_RES = 60;

	// Attack
	constexpr Milliseconds ATTACK_CD = 500.;
	const auto ATTACK_DAMAGE = Damage(FACTION, 200., 0., 200., 30.);
	constexpr double ATTACK_KNOCKBACK_X = 200. * 100.;
	constexpr double ATTACK_KNOCKBACK_Y = 150. * 100.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 150.;
	constexpr double AGGRO_RANGE_Y = 120.;
	constexpr double DEAGGRO_RANGE_X = 180.;
	constexpr double DEAGGRO_RANGE_Y = 160.;

	constexpr double HITBOX_OVERLAP_REQUIRED_TO_ATTACK = ct_sqr(8.);

	// Death
	constexpr int PARTICLE_COUNT = 10;
	constexpr double PARTICLE_MAX_SPEED_X = 200.;
	constexpr double PARTICLE_MAX_SPEED_Y = 200.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(3.);

}

m::enemy::Tentacle::Tentacle(const Vector2d &position) :
	Enemy(position)
{
	using namespace Tentacle_consts;

	// Init modules
	this->_init_sprite("[enemy]{tentacle}", { DEFAULT_ANIMATION_NAME, "emerge" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::ATTACK);
	this->_init_default_deaggroed_state(State::STAND);

	const double HITBOX_GAP = 13.;
	this->_optinit_healthbar_display(this->position, *this->health, Vector2d(0., -this->solid->hitboxSize.y / 2. - HITBOX_GAP));

	// Play emerging animation
	this->_sprite->animation_play("emerge");
	this->emerge_timer.start(this->_sprite->animation_duration("emerge"));

	// Prevent tentacles from being damaged as they emerge
	constexpr uint BIG_ENOUGH_HP = 100'000;
	this->health->setFlat(BIG_ENOUGH_HP, 0, 0, 0, 0);
}

bool m::enemy::Tentacle::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		emerge_timer.finished() &&
		std::abs(creatureRelativePos.x) < Tentacle_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < Tentacle_consts::AGGRO_RANGE_Y;
}

bool m::enemy::Tentacle::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > Tentacle_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > Tentacle_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::Tentacle::aggroTransition() {
	this->health->setFlat(0, 0, 0, 0, 0);

	this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
}

void m::enemy::Tentacle::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace Tentacle_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto sign = helpers::sign(this->target_relative_pos.x);

	const auto hitboxOverlapArea = this->solid->getHitbox().collideWithRect(this->target->solid->getHitbox()).overlap_area;

	// NOTE: Aggro'ed Tentacle has no state locks which is why there is no
	// state_isUnlocked() condition anywhere

	switch (currentState) {
	case State::ATTACK: {

		// Lifetime check
		if (this->lifetime.was_set() && this->lifetime.finished())
			this->health->instakill();

		// Target in range => attack
		if (this->attack_cd.finished() && hitboxOverlapArea > HITBOX_OVERLAP_REQUIRED_TO_ATTACK) {
			this->target->health->applyDamage(ATTACK_DAMAGE);
			this->target->solid->addImpulse_Horizontal(ATTACK_KNOCKBACK_X * sign);
			this->target->solid->addImpulse_Up(ATTACK_KNOCKBACK_Y);

			this->attack_cd.start(ATTACK_CD);
		}

		break;
	}
	default:
		break;
	}
}

void m::enemy::Tentacle::update_when_deaggroed(Milliseconds elapsedTime) {
	using namespace Tentacle_consts;

	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::STAND:

		// Lifetime check
		if (this->lifetime.was_set() && this->lifetime.finished())
			this->health->instakill();

		// After emerging switch to peristing animation
		if (this->_sprite->animation_finished())
			this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);

		break;

	default:
		break;
	}
}

void m::enemy::Tentacle::deathTransition() {
	Enemy::deathTransition();

	using namespace Tentacle_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}
}



// # BossMage1 #
namespace BossMage1_consts {

	const std::string BOSS_TITLE = "Head of the cult";

	// Next phase
	const std::string NEXT_PHASE_LEVEL = "lair_of_shadows";
	constexpr Vector2d NEXT_PHASE_PLAYER_POS = Vector2d(864, 494);

	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(52., 94.);
	constexpr auto SOLID_FLAGS = SolidFlags::NONE;
	constexpr double MASS = 250.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 9'800;
	constexpr sint REGEN = 20;
	constexpr sint PHYS_RES = 25;
	constexpr sint MAGIC_RES = 35;
	constexpr sint CHAOS_RES = 35;

	// Hover
	constexpr double VERTICAL_HOVER_HEIGHT = 8;
	constexpr double VERTICAL_HOVER_SPEED = 40;
	constexpr double VERTICAL_HOVER_ACCELERATION = 200;
	constexpr double VERTICAL_HOVER_FORCE = MASS * VERTICAL_HOVER_ACCELERATION;
	constexpr double VERTICAL_HOVER_EPSILON = 2;

	constexpr double HORIZONTAL_HOVER_MAX_DISTANCE = 32;
	constexpr double HORIZONTAL_HOVER_SPEED = 35;
	constexpr double HORIZONTAL_HOVER_ACCELERATION = 200;
	constexpr double HORIZONTAL_HOVER_FORCE = MASS * HORIZONTAL_HOVER_ACCELERATION;

	// Attack (summon tentacle)
	constexpr Milliseconds SUMMON_TENTACLE_CD_INITIAL = sec_to_ms(4.);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MIN = sec_to_ms(4.5);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MAX = sec_to_ms(7.5);

	constexpr int SUMMON_TENTACLE_COUNT_MIN = 2;
	constexpr int SUMMON_TENTACLE_COUNT_MAX = 5;

	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN = 16;
	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX = 80;

	constexpr double SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS = 12;
	constexpr double SUMMON_TENTACLE_MAX_SPAWN_TRIES = 3;

	constexpr Milliseconds SUMMON_TENTACLE_LIFETIME = sec_to_ms(6.);

	// Attack (summon skeleton halberd)
	constexpr Milliseconds SUMMON_SKELETON_CD_INITIAL = sec_to_ms(2.);
	constexpr Milliseconds SUMMON_SKELETON_CD_MIN = sec_to_ms(7.5);
	constexpr Milliseconds SUMMON_SKELETON_CD_MAX = sec_to_ms(15.5);

	constexpr double SUMMON_SKELETON_RANGE_FROM_CASTER_MIN = 8;
	constexpr double SUMMON_SKELETON_RANGE_FROM_CASTER_MAX = 16;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 50.;
	constexpr double DEAGGRO_RANGE_X = 400.;
	constexpr double DEAGGRO_RANGE_Y = 300.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	// Death
	constexpr Milliseconds DEATH_DELAY = sec_to_ms(2.); // so the boss doesn't visually disappear
}

m::enemy::BossMage1::BossMage1(const Vector2d& position) :
	Enemy(position)
{
	using namespace BossMage1_consts;

	// Init modules
	this->_init_sprite("[enemy]{boss_mage_phase_1}", { DEFAULT_ANIMATION_NAME });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::HOVER);
	this->_init_default_deaggroed_state(State::STAND);

	this->_optinit_boss_healthbar_display(*this->health, BOSS_TITLE);

	this->_optinit_death_delay(DEATH_DELAY);
}

bool m::enemy::BossMage1::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < BossMage1_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < BossMage1_consts::AGGRO_RANGE_Y;
}

bool m::enemy::BossMage1::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > BossMage1_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > BossMage1_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::BossMage1::aggroTransition() {
	using namespace BossMage1_consts;

	// Set up initial cd's so boss doesn't throw all available spells simultaneously at the beginning
	this->summon_tentacle_cd.start(SUMMON_TENTACLE_CD_INITIAL);
	this->summon_skeleton_cd.start(SUMMON_SKELETON_CD_INITIAL);
}

void m::enemy::BossMage1::deaggroTransition() {}

void m::enemy::BossMage1::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace BossMage1_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::HOVER: {

		// Look at the target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		// Hover to the target vertical level
		const double vertical_difference = this->target->solid->getHitbox().getBottom() - this->solid->getHitbox().getBottom() - VERTICAL_HOVER_HEIGHT;
			// positive value means the boss is higher, negative means the player is higher

		const bool vertical_difference_too_big = abs(vertical_difference) > VERTICAL_HOVER_EPSILON;
		const bool target_is_grounded = this->target->solid->is_grounded;

		if (vertical_difference_too_big && target_is_grounded) {

			if (vertical_difference > 0)
				this->solid->applyForceTillMaxSpeed_Down(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
			else
				this->solid->applyForceTillMaxSpeed_Up(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
		}
		else {
			this->solid->speed.y = 0;
		}

		// Hover away horizontaly
		const double combined_hitbox_halfwidth = 0.5 * (this->target->solid->getHitbox().getSizeX() + this->solid->getHitbox().getSizeX());
		const double horizontal_distance = abs(this->target_relative_pos.x) - combined_hitbox_halfwidth;
	
		if (horizontal_distance < HORIZONTAL_HOVER_MAX_DISTANCE)
			this->solid->applyForceTillMaxSpeed_Horizontal(HORIZONTAL_HOVER_FORCE, HORIZONTAL_HOVER_SPEED, invert(this->orientation));
		else
			this->solid->speed.x = 0;

		// Transition
		if (this->summon_tentacle_cd.finished()) {
			/*std::cout << "\n[TENTACLE]";*/
			this->state_change(State::SUMMON_TENTACLE);
		}

		if (this->summon_skeleton_cd.finished()) {
			/*std::cout << "\n[SKELETON]";*/
			this->state_change(State::SUMMON_SKELETON);
		}

		break;
	}
		
	case State::SUMMON_TENTACLE: {

		if (this->target->solid->is_grounded) {

			const int tentacle_count = rand_int(SUMMON_TENTACLE_COUNT_MIN, SUMMON_TENTACLE_COUNT_MAX);

			std::vector<double> tentacle_positions; // used to ensure min distance between tentacle spawns
			tentacle_positions.reserve(tentacle_count);

			for (int i = 0; i < tentacle_count; ++i) {
				// Create Tentacle entity
				auto spawned_tentacle = std::make_unique<m::enemy::Tentacle>(this->target->position);

				// Try finding a viable spawn location
				for (int spawn_try = 0; spawn_try < SUMMON_TENTACLE_MAX_SPAWN_TRIES; ++spawn_try) {
					// Select random spawn position (X) near the target
					const double rand_distance_from_target = rand_double(SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN, SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX);
					const double spawn_x = this->target->position.x + rand_distance_from_target * rand_choise({ -1, 1 });

					// Check that no other tentacle is too close
					bool spawn_allowed = true;
					for (const auto &pos : tentacle_positions) if (abs(spawn_x - pos) < SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS) {
						spawn_allowed = false;
						continue;
					}
					if (!spawn_allowed) continue;

					// Adjust spawn position (y) with respect to hitbox size
					const double ground_level = this->target->solid->getHitbox().getBottom();
					const double spawn_y = ground_level - 0.5 * spawned_tentacle->solid->getHitbox().getSizeY();

					// Spawn the creature
					spawned_tentacle->position.x = spawn_x;
					spawned_tentacle->position.y = spawn_y;
					spawned_tentacle->lifetime.start(SUMMON_TENTACLE_LIFETIME);

					Game::ACCESS->level->spawn(std::move(spawned_tentacle));
					tentacle_positions.push_back(spawn_x);

					// Stop further 'tries'
					break;
				}		
			}

			// Transition
			// (happens instantly, no conditions involved)
			this->summon_tentacle_cd.start(rand_double(SUMMON_TENTACLE_CD_MIN, SUMMON_TENTACLE_CD_MAX));
			this->state_change(State::HOVER);
		}

		break;
	}
	case State::SUMMON_SKELETON: {

		// Create SkeletonHalberd entity
		auto spawned_skeleton = std::make_unique<m::enemy::SkeletonHalberd>(this->target->position);

		// Select random spawn position (X) nearby
		const double rand_distance_from_caster = rand_double(SUMMON_SKELETON_RANGE_FROM_CASTER_MIN, SUMMON_SKELETON_RANGE_FROM_CASTER_MAX);
		const double spawn_x = this->position.x + rand_distance_from_caster * rand_choise({ -1., 1. });

		// Adjust spawn position (y) with respect to hitbox size
		const double ground_level = this->solid->getHitbox().getBottom();
		const double spawn_y = ground_level - 0.5 * spawned_skeleton->solid->getHitbox().getSizeY();

		// Spawn the creature
		spawned_skeleton->position.x = spawn_x;
		spawned_skeleton->position.y = spawn_y;
		Game::ACCESS->level->spawn(std::move(spawned_skeleton));

		// Transition
		// (happens instantly, no conditions involved)
		this->summon_skeleton_cd.start(rand_double(SUMMON_SKELETON_CD_MIN, SUMMON_SKELETON_CD_MAX));
		this->state_change(State::HOVER);
		break;
	}
	default:
		break;
	}
}

void m::enemy::BossMage1::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::BossMage1::deathTransition() {
	Enemy::deathTransition();

	using namespace BossMage1_consts;

	// Teleport player to a level with a next phase
	Game::ACCESS->request_levelChange(NEXT_PHASE_LEVEL, NEXT_PHASE_PLAYER_POS);
}



// # BossMage2 #
namespace BossMage2_consts {
	 
	const std::string BOSS_TITLE = "Head of the cult";

	// Next phase
	const std::string NEXT_PHASE_LEVEL = "deeper_realm";
	constexpr Vector2d NEXT_PHASE_PLAYER_POS = Vector2d(928, 336);

	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(52., 94.);
	constexpr auto SOLID_FLAGS = SolidFlags::NONE;
	constexpr double MASS = 250.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 23'900;
	constexpr sint REGEN = 0;
	constexpr sint PHYS_RES = 60;
	constexpr sint MAGIC_RES = 60;
	constexpr sint CHAOS_RES = 60;

	// Hover
	constexpr double VERTICAL_HOVER_SPEED = 40;
	constexpr double VERTICAL_HOVER_ACCELERATION = 200;
	constexpr double VERTICAL_HOVER_FORCE = MASS * VERTICAL_HOVER_ACCELERATION;
	constexpr double VERTICAL_HOVER_EPSILON = 2;

	constexpr double HORIZONTAL_HOVER_SPEED = 40;
	constexpr double HORIZONTAL_HOVER_ACCELERATION = 200;
	constexpr double HORIZONTAL_HOVER_FORCE = MASS * VERTICAL_HOVER_ACCELERATION;
	constexpr double HORIZONTAL_HOVER_EPSILON = 2;

	// Attack (summon tentacle)
	constexpr Milliseconds SUMMON_TENTACLE_CD_INITIAL = sec_to_ms(4.);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MIN = sec_to_ms(5.5);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MAX = sec_to_ms(8.5);

	constexpr int SUMMON_TENTACLE_COUNT_MIN = 1;
	constexpr int SUMMON_TENTACLE_COUNT_MAX = 2;

	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN = 16;
	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX = 80;

	constexpr Vector2d SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON = Vector2d(0, -4);
		// shift by which bottom hitbox corners are moved up when deducing their tile index
		// during spawn validity check (tentacles must have a tile underneath at both sides)
		/// Checking collisions would be better but cannot be done at spawn time
		/// Current checks are flawed and only work properly assuming available tiles are full-width
		/// and have a hitbox at the top

	constexpr double SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS = 12;
	constexpr double SUMMON_TENTACLE_MAX_SPAWN_TRIES = 3;

	constexpr Milliseconds SUMMON_TENTACLE_LIFETIME = sec_to_ms(6.);

	// Attack (summon fireballs)
	constexpr Milliseconds SUMMON_FIREBALL_CD_INITIAL = sec_to_ms(2.);
	constexpr Milliseconds SUMMON_FIREBALL_CD_MIN = sec_to_ms(0.5);
	constexpr Milliseconds SUMMON_FIREBALL_CD_MAX = sec_to_ms(1.5);

	constexpr int SUMMON_FIREBALL_COUNT_MIN = 1;
	constexpr int SUMMON_FIREBALL_COUNT_MAX = 9;

	constexpr double SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MIN = 4;
	constexpr double SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MAX = 20;

	constexpr double SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MIN = 10;
	constexpr double SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MAX = 120;

	constexpr double SUMMON_FIREBALL_MIN_DISTANCE_BETWEEN_SPAWNS = 10;
	constexpr double SUMMON_FIREBALL_MAX_SPAWN_TRIES = 3;

	// Fireball stats
	const auto FIREBALL_DAMAGE = Damage(FACTION, 0., 300., 100., 100.);
	constexpr double FIREBALL_KNOCKBACK = 120. * 100.;
	constexpr double FIREBALL_PROJECTILE_SPEED = 100.;
	constexpr Vector2d FIREBALL_AOE = Vector2d(20, 20);

	constexpr double FIREBALL_SPAWN_ALIGNMENT_Y = 12.;

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 80.;
	constexpr double DEAGGRO_RANGE_X = 400.;
	constexpr double DEAGGRO_RANGE_Y = 300.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	// Death
	constexpr Milliseconds DEATH_DELAY = sec_to_ms(2.);  // so the boss doesn't visually disappear
}

m::enemy::BossMage2::BossMage2(const Vector2d& position) :
	Enemy(position),
	anchor_position(position)
{
	using namespace BossMage2_consts;

	// Init modules
	this->_init_sprite("[enemy]{boss_mage_phase_2}", { DEFAULT_ANIMATION_NAME });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::HOVER);
	this->_init_default_deaggroed_state(State::STAND);

	this->_optinit_boss_healthbar_display(*this->health, BOSS_TITLE);

	this->_optinit_death_delay(DEATH_DELAY);
}

bool m::enemy::BossMage2::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < BossMage2_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < BossMage2_consts::AGGRO_RANGE_Y;
}

bool m::enemy::BossMage2::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > BossMage2_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > BossMage2_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::BossMage2::aggroTransition() {
	using namespace BossMage2_consts;

	// Set up initial cd's so boss doesn't throw all available spells simultaneously at the beginning
	this->summon_tentacle_cd.start(SUMMON_TENTACLE_CD_INITIAL);
	this->summon_fireball_cd.start(SUMMON_FIREBALL_CD_INITIAL);
}

void m::enemy::BossMage2::deaggroTransition() {}

void m::enemy::BossMage2::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace BossMage2_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::HOVER: {

		// Look at the target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		const auto displacement = this->position - this->anchor_position;

		// Hover to anchor (horizontally) 
		if (abs(displacement.x) > HORIZONTAL_HOVER_EPSILON) {
			if (displacement.x > 0)
				this->solid->applyForceTillMaxSpeed_Left(HORIZONTAL_HOVER_FORCE, HORIZONTAL_HOVER_SPEED);
			else
				this->solid->applyForceTillMaxSpeed_Right(HORIZONTAL_HOVER_FORCE, HORIZONTAL_HOVER_SPEED);
		}
		else {
			this->solid->speed.x = 0;
		}

		// Hover to anchor (vertically) 
		if (abs(displacement.y) > VERTICAL_HOVER_EPSILON) {
			if (displacement.y > 0)
				this->solid->applyForceTillMaxSpeed_Up(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
			else
				this->solid->applyForceTillMaxSpeed_Down(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
		}
		else {
			this->solid->speed.y = 0;
		}

		// Transition
		if (this->summon_tentacle_cd.finished()) {
			this->state_change(State::SUMMON_TENTACLE);
		}

		if (this->summon_fireball_cd.finished()) {
			this->state_change(State::SUMMON_FIREBALL);
		}

		break;
	}

	case State::SUMMON_TENTACLE: {

		if (this->target->solid->is_grounded) {

			const int tentacle_count = rand_int(SUMMON_TENTACLE_COUNT_MIN, SUMMON_TENTACLE_COUNT_MAX);

			std::vector<double> tentacle_positions; // used to ensure min distance between tentacle spawns
			tentacle_positions.reserve(tentacle_count);

			for (int i = 0; i < tentacle_count; ++i) {
				// Create Tentacle entity
				auto spawned_tentacle = std::make_unique<m::enemy::Tentacle>(this->target->position);

				// Try finding a viable spawn location
				for (int spawn_try = 0; spawn_try < SUMMON_TENTACLE_MAX_SPAWN_TRIES; ++spawn_try) {
					// Select random spawn position (X) near the target
					const double rand_distance_from_target = rand_double(SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN, SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX);
					const double spawn_x = this->target->position.x + rand_distance_from_target * rand_choise({ -1, 1 });

					spawned_tentacle->position.x = spawn_x;

					// Adjust spawn position (y) with respect to hitbox size
					const double ground_level = this->target->solid->getHitbox().getBottom();
					const double spawn_y = ground_level - 0.5 * spawned_tentacle->solid->getHitbox().getSizeY();

					spawned_tentacle->position.y = spawn_y;

					// Check that no other tentacle is too close
					bool spawn_allowed = true;
					for (const auto &pos : tentacle_positions) if (abs(spawn_x - pos) < SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS) {
						spawn_allowed = false;
						continue;
					}

					// Check that tentacle has ground underneath
					const Vector2d left_corner = spawned_tentacle->solid->getHitbox().getCornerBottomLeft() + SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON;
					const Vector2 tile_index_left_corner = helpers::divide32(left_corner);
					const bool tile_present_under_left = Game::READ->level->getTile(tile_index_left_corner.x, tile_index_left_corner.y + 1);

					const Vector2d right_corner = spawned_tentacle->solid->getHitbox().getCornerBottomRight() + SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON;
					const Vector2 tile_index_right_corner = helpers::divide32(right_corner);
					const bool tile_present_under_right = Game::READ->level->getTile(tile_index_right_corner.x, tile_index_right_corner.y + 1);

					if (spawn_allowed) std::cout
						<< "Tentacle [" << i << "]\n"
						<< "SPAWN TRY [" << spawn_try << "]\n"
						<< "tile_present_under_left = " << tile_present_under_left << "\n"
						<< "tile_present_under_right = " << tile_present_under_right << "\n";

					if (!tile_present_under_left || !tile_present_under_right) {
						spawn_allowed = false;
						continue;
					}

					if (!spawn_allowed) continue;

					// Spawn the creature
					spawned_tentacle->lifetime.start(SUMMON_TENTACLE_LIFETIME);

					Game::ACCESS->level->spawn(std::move(spawned_tentacle));
					tentacle_positions.push_back(spawn_x);

					// Stop further 'tries'
					break;
				}
			}

			// Transition
			// (happens instantly, no conditions involved)
			this->summon_tentacle_cd.start(rand_double(SUMMON_TENTACLE_CD_MIN, SUMMON_TENTACLE_CD_MAX));
			this->state_change(State::HOVER);
		}

		break;
	}
	case State::SUMMON_FIREBALL: {

		const int fireball_count = rand_int(SUMMON_FIREBALL_COUNT_MIN, SUMMON_FIREBALL_COUNT_MAX);

		std::vector<double> fireball_positions; // used to ensure min distance between fireball spawns
		fireball_positions.reserve(fireball_count);

		// Select 'window' near player that contains no fireballs
		const double window_x = this->target->position.x + rand_choise({ -1, 1 })
			* rand_double(SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MIN, SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MAX);

		const double camera_bottom = Graphics::READ->camera->get_FOV_rect().getBottom();

		for (int i = 0; i < fireball_count; ++i) {
			
			// Try finding a viable spawn location
			for (int spawn_try = 0; spawn_try < SUMMON_FIREBALL_MAX_SPAWN_TRIES; ++spawn_try) {
				// Select random spawn position (X) near the window
				const double rand_distance_from_target = rand_double(SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MIN, SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MAX);
				const double spawn_x = window_x + rand_distance_from_target * rand_choise({ -1, 1 });

				// Check that no other fireball is too close
				bool spawn_allowed = true;
				for (const auto &pos : fireball_positions) if (abs(spawn_x - pos) < SUMMON_FIREBALL_MIN_DISTANCE_BETWEEN_SPAWNS) {
					spawn_allowed = false;
					continue;
				}
				if (!spawn_allowed) continue;

				// Spawn projectile flying upwards from below the screen
				Game::ACCESS->level->spawn(std::make_unique<s::projectile::Fireball>(
					Vector2d(spawn_x, camera_bottom + FIREBALL_SPAWN_ALIGNMENT_Y),
					Vector2d(0., -FIREBALL_PROJECTILE_SPEED),
					FIREBALL_DAMAGE,
					FIREBALL_KNOCKBACK,
					FIREBALL_AOE)
				);

				fireball_positions.push_back(spawn_x);

				// Stop further 'tries'
				break;
			}
		}

		// Transition
		// (happens instantly, no conditions involved)
		this->summon_fireball_cd.start(rand_double(SUMMON_FIREBALL_CD_MIN, SUMMON_FIREBALL_CD_MAX));
		this->state_change(State::HOVER);
		break;
		
	}
	default:
		break;
	}
}

void m::enemy::BossMage2::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::BossMage2::deathTransition() {
	Enemy::deathTransition();

	using namespace BossMage2_consts;

	// Teleport player to a level with a next phase
	Game::ACCESS->request_levelChange(NEXT_PHASE_LEVEL, NEXT_PHASE_PLAYER_POS);
}



// # BossMage3 #
namespace BossMage3_consts {

	const std::string BOSS_TITLE = "Unleashed terror";

	// Next phase
	const std::string NEXT_PHASE_LEVEL = "lair_of_shadows_3";
	constexpr Vector2d NEXT_PHASE_PLAYER_POS = Vector2d(928, 336);

	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(52., 94.);
	constexpr auto SOLID_FLAGS = SolidFlags::NONE;
	constexpr double MASS = 250.;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 19'300;
	constexpr sint REGEN = 0;
	constexpr sint PHYS_RES = 60;
	constexpr sint MAGIC_RES = 60;
	constexpr sint CHAOS_RES = 60;

	// Hover
	constexpr double VERTICAL_HOVER_SPEED = 40;
	constexpr double VERTICAL_HOVER_ACCELERATION = 200;
	constexpr double VERTICAL_HOVER_FORCE = MASS * VERTICAL_HOVER_ACCELERATION;
	constexpr double VERTICAL_HOVER_EPSILON = 2;

	constexpr double HORIZONTAL_HOVER_SPEED = 40;
	constexpr double HORIZONTAL_HOVER_ACCELERATION = 200;
	constexpr double HORIZONTAL_HOVER_FORCE = MASS * VERTICAL_HOVER_ACCELERATION;
	constexpr double HORIZONTAL_HOVER_EPSILON = 2;

	// Attack (summon tentacle)
	constexpr Milliseconds SUMMON_TENTACLE_CD_INITIAL = sec_to_ms(4.);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MIN = sec_to_ms(5.5);
	constexpr Milliseconds SUMMON_TENTACLE_CD_MAX = sec_to_ms(8.5);

	constexpr int SUMMON_TENTACLE_COUNT_MIN = 2;
	constexpr int SUMMON_TENTACLE_COUNT_MAX = 5;

	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN = 2;
	constexpr double SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX = 100;

	constexpr Vector2d SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON = Vector2d(0, -4);
		// shift by which bottom hitbox corners are moved up when deducing their tile index
		// during spawn validity check (tentacles must have a tile underneath at both sides)
		/// Checking collisions would be better but cannot be done at spawn time
		/// Current checks are flawed and only work properly assuming available tiles are full-width
		/// and have a hitbox at the top

	constexpr double SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS = 12;
	constexpr double SUMMON_TENTACLE_MAX_SPAWN_TRIES = 3;

	constexpr Milliseconds SUMMON_TENTACLE_LIFETIME = sec_to_ms(6.);

	// Attack (summon fireballs)
	constexpr Milliseconds SUMMON_FIREBALL_CD_INITIAL = sec_to_ms(2.);
	constexpr Milliseconds SUMMON_FIREBALL_CD_MIN = sec_to_ms(1.5);
	constexpr Milliseconds SUMMON_FIREBALL_CD_MAX = sec_to_ms(1.9);

	constexpr int SUMMON_FIREBALL_COUNT_MIN = 7;
	constexpr int SUMMON_FIREBALL_COUNT_MAX = 12;

	constexpr double SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MIN = 2;
	constexpr double SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MAX = 8;

	constexpr double SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MIN = 16;
	constexpr double SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MAX = 120;

	constexpr double SUMMON_FIREBALL_MIN_DISTANCE_BETWEEN_SPAWNS = 10;
	constexpr double SUMMON_FIREBALL_MAX_SPAWN_TRIES = 3;

	// Fireball stats
	const auto FIREBALL_DAMAGE = Damage(FACTION, 0., 300., 100., 100.);
	constexpr double FIREBALL_KNOCKBACK = 120. * 100.;
	constexpr double FIREBALL_PROJECTILE_SPEED = 100.;
	constexpr Vector2d FIREBALL_AOE = Vector2d(20, 20);

	constexpr double FIREBALL_SPAWN_ALIGNMENT_Y = 12.;

	// Attacks (summon circle of spirit bombs)
	constexpr Milliseconds SUMMON_BOMBS_CD_INITIAL = sec_to_ms(7.);
	constexpr Milliseconds SUMMON_BOMBS_CD_MIN = sec_to_ms(6.5);
	constexpr Milliseconds SUMMON_BOMBS_CD_MAX = sec_to_ms(9.5);

	constexpr int SUMMON_BOMBS_COUNT = 32;

	constexpr Milliseconds SUMMON_BOMBS_DELAY = sec_to_ms(1.2);
	constexpr double SUMMON_BOMBS_DISTANCE_FROM_CASTER = 36;

	// Spirit bomb stats
	constexpr double BOMB_RADIAL_SPEED = 100;

	constexpr auto BOMB_DAMAGE = Damage(FACTION, 0., 500., 0., 250);
	constexpr double BOMB_KNOCKBACK = 110. * 100.;
	constexpr Vector2d BOMB_AOE = Vector2d(20, 20);

	// Behaviour
	constexpr double AGGRO_RANGE_X = 220.;
	constexpr double AGGRO_RANGE_Y = 80.;
	constexpr double DEAGGRO_RANGE_X = 400.;
	constexpr double DEAGGRO_RANGE_Y = 300.;

	constexpr double ORIENTAION_CHANGE_RANGE = 10.; // don't change orientation if target closer than that

	// Death
	constexpr int PARTICLE_COUNT = 160;
	constexpr double PARTICLE_MAX_SPEED_X = 400.;
	constexpr double PARTICLE_MAX_SPEED_Y = 450.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(2.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);
}

m::enemy::BossMage3::BossMage3(const Vector2d& position) :
	Enemy(position),
	anchor_position(position)
{
	using namespace BossMage3_consts;

	// Init modules
	this->_init_sprite("[enemy]{boss_mage_phase_3}", { DEFAULT_ANIMATION_NAME });

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
		CHAOS_RES
	);

	// Init members
	this->_init_default_aggroed_state(State::HOVER);
	this->_init_default_deaggroed_state(State::STAND);

	this->_optinit_boss_healthbar_display(*this->health, BOSS_TITLE);
}

bool m::enemy::BossMage3::aggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) < BossMage3_consts::AGGRO_RANGE_X &&
		std::abs(creatureRelativePos.y) < BossMage3_consts::AGGRO_RANGE_Y;
}

bool m::enemy::BossMage3::deaggroCondition(Creature* creature) {
	const auto creatureRelativePos = this->position - creature->position;

	return
		std::abs(creatureRelativePos.x) > BossMage3_consts::DEAGGRO_RANGE_X ||
		std::abs(creatureRelativePos.y) > BossMage3_consts::DEAGGRO_RANGE_Y;
}

void m::enemy::BossMage3::aggroTransition() {
	using namespace BossMage3_consts;

	// Set up initial cd's so boss doesn't throw all available spells simultaneously at the beginning
	this->summon_tentacle_cd.start(SUMMON_TENTACLE_CD_INITIAL);
	this->summon_fireball_cd.start(SUMMON_FIREBALL_CD_INITIAL);
	this->summon_bombs_cd.start(SUMMON_BOMBS_CD_INITIAL);
}

void m::enemy::BossMage3::deaggroTransition() {}

void m::enemy::BossMage3::update_when_aggroed(Milliseconds elapsedTime) {
	using namespace BossMage3_consts;

	const auto currentState = static_cast<State>(this->state_get());

	const auto targetPosSign = sign(this->target_relative_pos.x);

	switch (currentState) {
	case State::HOVER: {

		// Look at the target
		if (std::abs(this->target_relative_pos.x) > ORIENTAION_CHANGE_RANGE)
			this->orientation = targetPosSign < 0 ? Orientation::LEFT : Orientation::RIGHT;

		const auto displacement = this->position - this->anchor_position;

		// Hover to anchor (horizontally) 
		if (abs(displacement.x) > HORIZONTAL_HOVER_EPSILON) {
			if (displacement.x > 0)
				this->solid->applyForceTillMaxSpeed_Left(HORIZONTAL_HOVER_FORCE, HORIZONTAL_HOVER_SPEED);
			else
				this->solid->applyForceTillMaxSpeed_Right(HORIZONTAL_HOVER_FORCE, HORIZONTAL_HOVER_SPEED);
		}
		else {
			this->solid->speed.x = 0;
		}

		// Hover to anchor (vertically) 
		if (abs(displacement.y) > VERTICAL_HOVER_EPSILON) {
			if (displacement.y > 0)
				this->solid->applyForceTillMaxSpeed_Up(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
			else
				this->solid->applyForceTillMaxSpeed_Down(VERTICAL_HOVER_FORCE, VERTICAL_HOVER_SPEED);
		}
		else {
			this->solid->speed.y = 0;
		}

		// Transition
		if (this->summon_tentacle_cd.finished()) {
			this->state_change(State::SUMMON_TENTACLE);
		}

		if (this->summon_fireball_cd.finished()) {
			this->state_change(State::SUMMON_FIREBALL);
		}

		if (this->summon_bombs_cd.finished()) {
			this->state_change(State::SUMMON_BOMBS);
		}

		break;
	}

	case State::SUMMON_TENTACLE: {

		if (this->target->solid->is_grounded) {

			const int tentacle_count = rand_int(SUMMON_TENTACLE_COUNT_MIN, SUMMON_TENTACLE_COUNT_MAX);

			std::vector<double> tentacle_positions; // used to ensure min distance between tentacle spawns
			tentacle_positions.reserve(tentacle_count);

			for (int i = 0; i < tentacle_count; ++i) {
				// Create Tentacle entity
				auto spawned_tentacle = std::make_unique<m::enemy::Tentacle>(this->target->position);

				// Try finding a viable spawn location
				for (int spawn_try = 0; spawn_try < SUMMON_TENTACLE_MAX_SPAWN_TRIES; ++spawn_try) {
					// Select random spawn position (X) near the target
					const double rand_distance_from_target = rand_double(SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MIN, SUMMON_TENTACLE_RANGE_AWAY_FROM_PLAYER_MAX);
					const double spawn_x = this->target->position.x + rand_distance_from_target * rand_choise({ -1, 1 });

					spawned_tentacle->position.x = spawn_x;

					// Adjust spawn position (y) with respect to hitbox size
					const double ground_level = this->target->solid->getHitbox().getBottom();
					const double spawn_y = ground_level - 0.5 * spawned_tentacle->solid->getHitbox().getSizeY();

					spawned_tentacle->position.y = spawn_y;

					// Check that no other tentacle is too close
					bool spawn_allowed = true;
					for (const auto &pos : tentacle_positions) if (abs(spawn_x - pos) < SUMMON_TENTACLE_MIN_DISTANCE_BETWEEN_SPAWNS) {
						spawn_allowed = false;
						continue;
					}

					// Check that tentacle has ground underneath
					const Vector2d left_corner = spawned_tentacle->solid->getHitbox().getCornerBottomLeft() + SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON;
					const Vector2 tile_index_left_corner = helpers::divide32(left_corner);
					const bool tile_present_under_left = Game::READ->level->getTile(tile_index_left_corner.x, tile_index_left_corner.y + 1);

					const Vector2d right_corner = spawned_tentacle->solid->getHitbox().getCornerBottomRight() + SUMMON_TENTACLE_VALIDITY_CHECK_EPSILON;
					const Vector2 tile_index_right_corner = helpers::divide32(right_corner);
					const bool tile_present_under_right = Game::READ->level->getTile(tile_index_right_corner.x, tile_index_right_corner.y + 1);

					if (!tile_present_under_left || !tile_present_under_right) {
						spawn_allowed = false;
						continue;
					}

					if (!spawn_allowed) continue;

					// Spawn the creature
					spawned_tentacle->lifetime.start(SUMMON_TENTACLE_LIFETIME);

					Game::ACCESS->level->spawn(std::move(spawned_tentacle));
					tentacle_positions.push_back(spawn_x);

					// Stop further 'tries'
					break;
				}
			}

			// Transition
			// (happens instantly, no conditions involved)
			this->summon_tentacle_cd.start(rand_double(SUMMON_TENTACLE_CD_MIN, SUMMON_TENTACLE_CD_MAX));
			this->state_change(State::HOVER);
		}

		break;
	}
	case State::SUMMON_FIREBALL: {

		const int fireball_count = rand_int(SUMMON_FIREBALL_COUNT_MIN, SUMMON_FIREBALL_COUNT_MAX);

		std::vector<double> fireball_positions; // used to ensure min distance between fireball spawns
		fireball_positions.reserve(fireball_count);

		// Select 'window' near player that contains no fireballs
		const double window_x = this->target->position.x + rand_choise({ -1, 1 })
			* rand_double(SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MIN, SUMMON_FIREBALL_WINDOW_DISTANCE_FROM_PLAYER_MAX);

		const double camera_bottom = Graphics::READ->camera->get_FOV_rect().getBottom();

		for (int i = 0; i < fireball_count; ++i) {

			// Try finding a viable spawn location
			for (int spawn_try = 0; spawn_try < SUMMON_FIREBALL_MAX_SPAWN_TRIES; ++spawn_try) {
				// Select random spawn position (X) near the window
				const double rand_distance_from_target = rand_double(SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MIN, SUMMON_FIREBALL_RANGE_AWAY_FROM_WINDOW_MAX);
				const double spawn_x = window_x + rand_distance_from_target * rand_choise({ -1, 1 });

				// Check that no other fireball is too close
				bool spawn_allowed = true;
				for (const auto &pos : fireball_positions) if (abs(spawn_x - pos) < SUMMON_FIREBALL_MIN_DISTANCE_BETWEEN_SPAWNS) {
					spawn_allowed = false;
					continue;
				}
				if (!spawn_allowed) continue;

				// Spawn projectile flying upwards from below the screen
				Game::ACCESS->level->spawn(std::make_unique<s::projectile::Fireball>(
					Vector2d(spawn_x, camera_bottom + FIREBALL_SPAWN_ALIGNMENT_Y),
					Vector2d(0., -FIREBALL_PROJECTILE_SPEED),
					FIREBALL_DAMAGE,
					FIREBALL_KNOCKBACK,
					FIREBALL_AOE)
				);

				fireball_positions.push_back(spawn_x);

				// Stop further 'tries'
				break;
			}
		}

		// Transition
		// (happens instantly, no conditions involved)
		this->summon_fireball_cd.start(rand_double(SUMMON_FIREBALL_CD_MIN, SUMMON_FIREBALL_CD_MAX));
		this->state_change(State::HOVER);
		break;

	}
	case State::SUMMON_BOMBS: {

		Vector2d direction = Vector2d(1, 0);
		const double angle_increment = 2. * helpers::PI / SUMMON_BOMBS_COUNT;

		for (int i = 0; i < SUMMON_BOMBS_COUNT; ++i) { 
			direction.rotate(angle_increment);

			auto spawned_bomb = std::make_unique<s::projectile::SpiritBomb>(
					this->position + direction * SUMMON_BOMBS_DISTANCE_FROM_CASTER,
					direction * BOMB_RADIAL_SPEED,
					BOMB_DAMAGE,
					BOMB_KNOCKBACK,
					BOMB_AOE
				);

			spawned_bomb->delay.start(SUMMON_BOMBS_DELAY);

			Game::ACCESS->level->spawn(std::move(spawned_bomb));
		}

		// Transition
		// (happens instantly, no conditions involved)
		this->summon_bombs_cd.start(rand_double(SUMMON_BOMBS_CD_MIN, SUMMON_BOMBS_CD_MAX));
		this->state_change(State::HOVER);
		break;
	}
	default:
		break;
	}
}

void m::enemy::BossMage3::update_when_deaggroed(Milliseconds elapsedTime) {
	// Stand idle when deaggroed
}

void m::enemy::BossMage3::deathTransition() {
	Enemy::deathTransition();

	using namespace BossMage3_consts;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}

	// Go to ending screen, the game is finished
	Game::ACCESS->request_goToEndingScreen();
}


/* ### item_entity:: ### */

// # EldritchBattery #
m::item_entity::EldritchBattery::EldritchBattery(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(9., 13.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{eldritch_battery}");

	this->name = "eldritch_battery";
}

// # PowerShard #
m::item_entity::PowerShard::PowerShard(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(12., 12.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{power_shard}");

	this->name = "power_shard";
}

// # SpiderSignet #
m::item_entity::SpiderSignet::SpiderSignet(const Vector2d &position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(14., 11.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{spider_signet}");

	this->name = "spider_signet";
}

// # BoneMask #
m::item_entity::BoneMask::BoneMask(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(16., 16.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{bone_mask}");

	this->name = "bone_mask";
}

// # MagicNegator #
m::item_entity::MagicNegator::MagicNegator(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(16., 16.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{magic_negator}");

	this->name = "magic_negator";
}

// # TwinSouls #
m::item_entity::TwinSouls::TwinSouls(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(16., 16.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{twin_souls}");

	this->name = "twin_souls";
}

// # WatchingEye #
m::item_entity::WatchingEye::WatchingEye(const Vector2d& position) :
	ItemEntity(position)
{
	constexpr auto HITBOX_SIZE = Vector2d(14., 14.);
	this->_init_solid(HITBOX_SIZE);
	this->_init_sprite(false, "[item]{watching_eye}");

	this->name = "watching_eye";
}



/* ### destructible:: ### */

// # OrbOfBetrayal #
namespace OrbOfBetrayal_consts {
	// Physics
	constexpr auto HITBOX_SIZE = Vector2d(10., 16.);
	constexpr auto SOLID_FLAGS = SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY;
	constexpr double MASS = 1e6;
	constexpr double FRICTION = 0.95;

	// Stats
	constexpr Faction FACTION = Faction::SHADOW;
	constexpr uint MAX_HP = 1'000;
	constexpr sint REGEN = 20;
	constexpr sint PHYS_RES = 0;
	constexpr sint MAGIC_RES = 0;
	constexpr sint CHAOS_RES = 0;

	constexpr auto DAMAGE = Damage(Faction::PROPS, 0., 0., 0., 3000.);

	const double AOE_RADIUS = 160;
	const double AOE_RADIUS_2 = AOE_RADIUS * AOE_RADIUS;
}

m::destructible::OrbOfBetrayal::OrbOfBetrayal(const Vector2d &position) :
	Destructible(position)
{
	using namespace OrbOfBetrayal_consts;

	// Init modules
	this->_init_sprite("[destructible]{orb_of_betrayal}", { DEFAULT_ANIMATION_NAME, "explosion" });

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
		CHAOS_RES
	);

	// Init members
	this->_init_delay(this->_sprite->animation_duration("explosion"));
}

void m::destructible::OrbOfBetrayal::effect() {
	using namespace OrbOfBetrayal_consts;

	// Damage creatures of the same faction that are in the radius
	for (const auto& entity : Game::ACCESS->level->entities_type[TypeId::CREATURE]) {
		const auto creature = static_cast<ntt::m_type::Creature*>(entity); // we 100% know that it's creature

		const bool factions_are_same = (creature->health->faction == this->health->faction);
		const bool in_radius = (this->position - creature->position).length2() < AOE_RADIUS_2;

		if (factions_are_same && in_radius) {
			creature->health->applyDamage(DAMAGE);
		}
	}

	// Play explosion animation
	this->_sprite->animation_play("explosion");
}