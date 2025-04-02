#pragma once

#include "entity_type_m.h" // 'Creature' base class
#include "inventory.h" // 'Inventory' module



namespace artifacts {

	// Regen
	constexpr double ELDRITCH_BATTERY_REGEN_BOOST = 0.10;

	// Damage
	constexpr double POWER_SHARD_DMG_BOOST = 0.1;

	// Utility
	constexpr double SPIDER_SIGNET_JUMP_BOOST = 0.08;

	// Resistances
	constexpr double BONE_MASK_PHYS_DMG_REDUCTION = 0.2;
	constexpr double MAGIC_NEGATOR_MAGIC_DMG_REDUCTION = 0.2;
	constexpr double TWIN_SOULS_CHAOS_DMG_REDUCTION = 0.2;
}



// ntt::player::
// - Contains player class
// - Unlike other creatures doesn't get erased upon death, but rather 'fakes' the effects of it
namespace ntt::player {
	// # Player #
	class Player : public m_type::Creature {
	public:
		Player() = delete;

		Player(const Vector2d &position);

		virtual ~Player() = default;

		TypeId type_id() const override;

		bool update(Milliseconds elapsedTime) override;

		enum class State {
			STAND,
			MOVE,
			ATTACK,
			SKILL
		};

		void update_charges(Milliseconds elapsedTime);

		void update_case_STAND(Milliseconds elapsedTime);
		void update_case_MOVE(Milliseconds elapsedTime);
		void update_case_ATTACK(Milliseconds elapsedTime);
		void update_case_ULT(Milliseconds elapsedTime);

		void jump();
		void jump_down_start(); // jump down the platform
		void jump_down_end();
		void horizontal_blink(Orientation direction, double range); // teleport with respect to terrain

		void update_cameraTrapPos(Milliseconds elapsedTime);

		void draw() const override; // also draws effect_sprite

		void deathTransition() override;

		const Vector2d& cameraTrap_getPosition() const;
		void cameraTrap_center(); // centers trap at the player, used during level change 

		std::string get_state_name(); // used in debug display

		Inventory inventory;

		uint charges_current;
		uint charges_max;
		Milliseconds charges_time_elapsed;

	private:
		void _init_effect_sprite(const std::string &folder, std::initializer_list<std::string> animationNames);

		void _recalculate_stats(); // every update recalculate stats base on artifacts

		std::unique_ptr<HealthbarDisplay> healthbar_display;

		Vector2d camera_trap_pos;
		Vector2d camera_pos;

		int chain_progress; // index of current attack in chain

		std::unique_ptr<ControllableSprite> effect_sprite;
			// secondary sprite used to render effects (like powerfull jump) without
			// interfering with main animations

		bool death_transition_performed;

		Timer dropping_down_sticky_delay; // delays setting 'solid->is_dropping_down' after player releases 'S'
			// which prevents characted from teleporting slightly up after releasing 'S' too fast
	};
}