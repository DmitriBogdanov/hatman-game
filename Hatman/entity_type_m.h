#pragma once

/*
Suffix _m stands for 'makable' entities that:
- Take only position in their constructors
- Can be parsed from a map file
- Have predetermined values upon creation
*/

#include "entity_base.h" // 'Entity' base class
#include "inventory.h" // 'Inventory' module (player forms)



// ntt::m_type::
// - 'm' stand for 'makable' - aka entities can be parsed from a map file
// - Contains all types of 'm' entities
namespace ntt::m_type {
	// # Creature #
	// - Entity with conrollable sprite, solid and health
	// - Has a LEFT/RIGHT orientation
	class Creature : public Entity {
	public:
		Creature() = delete;

		Creature(const Vector2d &position);

		virtual ~Creature() = default;

		TypeId type_id() const override;

		virtual bool update(Milliseconds elapsedTime) override;

		Orientation orientation = Orientation::RIGHT; // flips sprite automatically

	protected:
		int state_get() const;
		bool state_isLocked() const;
		bool state_isUnlocked() const;

		void state_lock(Milliseconds duration = MAX_POSSIBLE_TIME); // prevents state change for duration, locks forever by default
		void state_unlock();

		template<typename StateEnum>
		void state_tryChange(StateEnum newState) {
			if (this->state_lock_timer.finished()) { this->state = static_cast<int>(newState); }
		}

		template<typename StateEnum> // ignores state lock
		void state_change(StateEnum newState) {
			this->state = static_cast<int>(newState);
		}

		// Module inits
		void _init_sprite(const std::string &folder, std::initializer_list<std::string> animationNames);
		void _init_solid(const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction);
		void _init_health(Faction faction, uint maxHp, sint regen, sint physRes = 0, sint magicRes = 0, sint dotRes = 0);

		virtual void deathTransition(); // called when hp reaches 0

		ControllableSprite* _sprite; // casted version of 'sprite', used to access methods in 'ControllableSprite'

		bool creature_is_alive;
	private:
		// State can only be operated through state_tryChange() and state_lock()
		int state;
		Timer state_lock_timer;	
	};



	// # Enemy #
	// - Agressive creature that only attacks player
	class Enemy : public Creature {
	public:
		Enemy() = delete;

		Enemy(const Vector2d &position);

		virtual ~Enemy() = default;

		TypeId type_id() const override;

		bool update(Milliseconds elapsedTime) override;

		void draw() const override; // also draws HealthbarDisplay

	protected:
		Creature* target; // nullptr when enemy is not aggro'ed
		Vector2d target_relative_pos; // store here so we don't have to recalculate it constantly

		virtual bool aggroCondition(Creature* creature) = 0;
		virtual bool deaggroCondition(Creature* creature) = 0;

		virtual void aggroTransition(); // does nothing, implement if additional behaviour is needed
		virtual void deaggroTransition();

		// for simplifying state machine we separate it into 2 parts: aggro'ed and idle
		virtual void update_when_aggroed(Milliseconds elapsedTime) = 0;
		virtual void update_when_deaggroed(Milliseconds elapsedTime) = 0;	

		// Effects
		virtual void deathTransition() override; // dies after a delay

		// Module inits, call after 'Creature' inits are done
		template<typename StateEnum>
		void _init_default_aggroed_state(StateEnum state) {
			this->default_aggroed_state = static_cast<int>(state);
		}

		template<typename StateEnum>
		void _init_default_deaggroed_state(StateEnum state) {
			this->default_deaggroed_state = static_cast<int>(state);
		}

		void _optinit_healthbar_display(const Vector2d &parentPosition, const Health &parentHealth, const Vector2d &bottomCenterpointAlignment);
	
		void _optinit_death_delay(Milliseconds delay);
	private:
		std::unique_ptr<HealthbarDisplay> healthbar_display; // it's assumed that 'Health' module is always present

		int default_aggroed_state;
		int default_deaggroed_state;

		Milliseconds death_delay;
	};



	// # ItemEntity #
	// - Entity used to represent items outside of inventories
	// - Can be picked up by the player
	class ItemEntity : public Entity {
	public:
		ItemEntity() = delete;

		ItemEntity(const Vector2d &position);

		virtual ~ItemEntity() = default;

		TypeId type_id() const override;

		bool update(Milliseconds elapsedTime) override;

	protected:
		std::string name;

		// Checks
		virtual bool checkActivation() const;  // checks if item should be activated
		virtual bool checkTrigger() const; // checks if trigger condition is true (item must be ativated)

		// Actions
		virtual void activate(); // triggers the item active state
		virtual void trigger(); // triggers item effect (pickup by default)

		// Module inits
		void _init_sprite(bool animated, const std::string &folder, const std::string &filename = DEFAULT_ANIMATION_NAME);
		void _init_solid(const Vector2d &hitboxSize);
			// inits solid with standard flags

		// Member inits
		void _init_name(const std::string &name);
	};



	// # Destructible #
	// - Entity with lifebar that triggers some effect upon death
	class Destructible : public Entity { /// NEEDS REWORK
	public:
		Destructible() = delete;

		Destructible(const Vector2d &position);

		virtual ~Destructible() = default;

		TypeId type_id() const override;

		bool update(Milliseconds elapsedTime) override;

	protected:
		bool effect_triggered = false;
		Milliseconds erasion_delay; // delay before entity is erased after its death (ie time for death animation) 
		Timer timer; // handles erasion delay

		// Effects
		virtual void effect(); // any effect that is triggered upon entity death

		// Module inits
		void _init_solid(const Vector2d &hitboxSize);
			// inits solid with standard flags
		void _init_health(Faction faction, uint maxHp, sint regen = 0, sint physRes = 0, sint magicRes = 0, sint dotRes = 0);

		// Member inits
		void _init_delay(Milliseconds erasionDelay);

	private:
		ControllableSprite* _sprite; // casted version of 'sprite', used to access methods in 'ControllableSprite'
	};
}