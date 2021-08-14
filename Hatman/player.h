#pragma once

#include "entity_type_m.h" // 'Creature' base class
#include "inventory.h" // 'Inventory' module



// ntt::player::
// - Contains player class
// - Contains forms (forms themselves are not entities but are in a sense extension of the player)
namespace ntt::player {
	// # Player #
	// - Holds methods specific to player, but not the particular form
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

		enum class Attunement {
			FIRE,
			AIR,
			WATER,
			EARTH
		};

		void update_case_STAND(Milliseconds elapsedTime);
		void update_case_MOVE(Milliseconds elapsedTime);
		void update_case_ATTACK(Milliseconds elapsedTime);
		void update_case_ULT(Milliseconds elapsedTime);

		void jump();
		void jump_down_start(); // jump down the platform
		void jump_down_end();
		void horizontal_blink(Orientation direction, double range); // teleport with respect to terrain

		void update_cameraTrapPos(Milliseconds elapsedTime);

		void draw() const override; // also draws Player_Form

		void deathTransition() override;

		const Vector2d& cameraTrap_getPosition() const;
		void cameraTrap_center(); // centers trap at the player, used during level change 

		std::string get_state_name(); // used in debug display

		Inventory inventory;

	private:
		Vector2d camera_trap_pos;
		Vector2d camera_pos;

		Attunement attunement;
		int chain_progress; // index of current attack in chain

		///std::unique_ptr<ControllableSprite> attunement_sprite;
			// holds parts of the character that change color depending on current attunement
	};
}