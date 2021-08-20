#pragma once

/*
Suffix _m stands for 'makable' entities that:
- Take only position in their constructors
- Can be parsed from a map file using make_entity() function
- Have predetermined values upon creation
*/

#include "entity_type_m.h" // 'entity_type_m' base classes



// ntt::m::
// - 'm' stand for 'makable' - aka entities can be parsed from a map file
// - Contains all unique 'm' entities
namespace ntt::m {
	std::unique_ptr<Entity> make_entity(const std::string &type, const std::string &name, const Vector2d &position);
		// creates entity of a correct class based on type and name and returns ownership


	namespace enemy {
		// # Ghost #
		// - Fast agressive movement
		// - Fast melee attacks
		// - Ignores terrain
		/// WORK TO DO



		// # Sludge #
		class Sludge : public m_type::Enemy {
		public:
			Sludge() = delete;

			Sludge(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
		};



		// # SkeletonHalberd #
		class SkeletonHalberd : public m_type::Enemy {
		public:
			SkeletonHalberd() = delete;

			SkeletonHalberd(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				ATTACK_WINDUP,
				ATTACK_RECOVER
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;

			Timer wander_timer; // used to make random turns while wandering
			bool wander_move; // decides whether enemy will more or stand still for the timer duration
		};



		// # Devourer #
		// - Fast enemy that attacks only when player looks in opposite direction
		class Devourer : public m_type::Enemy {
		public:
			Devourer() = delete;

			Devourer(const Vector2d& position);

		private:
			enum class State {
				STAND,
				CHASE
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
			Milliseconds time_target_was_looking_away;
			Milliseconds time_target_was_looking_towards;
		};
	}



	namespace item_entity {
		// # BrassRelic #
		struct BrassRelic : public m_type::ItemEntity {
			BrassRelic() = delete;

			BrassRelic(const Vector2d &position);
		};



		// # Paper #
		struct Paper : public m_type::ItemEntity {
			Paper() = delete;

			Paper(const Vector2d &position);
		};
	}



	namespace destructible {
		// # TNT #
		class TNT : public m_type::Destructible {
		public:
			TNT() = delete;

			TNT(const Vector2d &position);

		private:
			void effect() override;
		};
	}
}
