#pragma once

/*
Suffix _m stands for 'makable' entities that:
- Take only position in their constructors
- Can be parsed from a map file using make_entity() function
- Have predetermined values upon creation
*/

#include "entity/type_m.h" // 'entity_type_m' base classes



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
				AWAIT,
				ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
		};



		// # Worm #
		class Worm : public m_type::Enemy {
		public:
			Worm() = delete;

			Worm(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				AWAIT,
				ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
		};


		// # Golem #
		class Golem : public m_type::Enemy {
		public:
			Golem() = delete;

			Golem(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				AWAIT,
				ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

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
				AWAIT,
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



		// # PygmyWarrior #
		class PygmyWarrior : public m_type::Enemy {
		public:
			PygmyWarrior() = delete;

			PygmyWarrior(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				AWAIT,
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



		// # SpiritBomber #
		class SpiritBomber : public m_type::Enemy {
		public:
			SpiritBomber() = delete;

			SpiritBomber(const Vector2d& position);

		private:
			enum class State {
				STAND,
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
		};



		// # CultistMage #
		class CultistMage : public m_type::Enemy {
		public:
			CultistMage() = delete;

			CultistMage(const Vector2d& position);

		private:
			enum class State {
				STAND,
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
		};



		// # Hellhound #
		class Hellhound : public m_type::Enemy {
		public:
			Hellhound() = delete;

			Hellhound(const Vector2d &position);

		private:
			enum class State {
				WANDER_STAND,
				WANDER_MOVE,
				CHASE,
				FLEE,
				AWAIT,
				JUMP_ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
			Timer flee_timer;

			Timer wander_timer; // used to make random turns while wandering
			bool wander_move; // decides whether enemy will more or stand still for the timer duration
		};



		// # Necromancer #
		class Necromancer : public m_type::Enemy {
		public:
			Necromancer() = delete;

			Necromancer(const Vector2d &position);

		private:
			enum class State {
				STAND,
				ATTACK // summons creatures
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
		};



		// # Tentacle #
		class Tentacle : public m_type::Enemy {
		public:
			Tentacle() = delete;

			Tentacle(const Vector2d &position);

			Timer lifetime; // if timer wasn't set then it's duration is negative which means lifetime is not limited

		private:
			enum class State {
				EMERGE,
				STAND,
				ATTACK
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer attack_cd;
			Timer emerge_timer;
		};



		// # MagesBoss (Phase 1) #
		class BossMage1 : public m_type::Enemy {
		public:
			BossMage1() = delete;

			BossMage1(const Vector2d &position);

		private:
			enum class State {
				STAND,
				HOVER,
				SUMMON_TENTACLE,
				SUMMON_SKELETON
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Timer summon_tentacle_cd;
			Timer summon_skeleton_cd;
		};

		// # MagesBoss (Phase 2) #
		class BossMage2 : public m_type::Enemy {
		public:
			BossMage2() = delete;

			BossMage2(const Vector2d &position);

		private:
			enum class State {
				STAND,
				HOVER,
				SUMMON_TENTACLE,
				SUMMON_FIREBALL
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Vector2d anchor_position; // boss always floats to it's initial spawn pos, being effectively stationary

			Timer summon_tentacle_cd;
			Timer summon_fireball_cd;
		};

		// # MagesBoss (Phase 3) #
		class BossMage3 : public m_type::Enemy {
		public:
			BossMage3() = delete;

			BossMage3(const Vector2d &position);

		private:
			enum class State {
				STAND,
				HOVER,
				SUMMON_TENTACLE,
				SUMMON_FIREBALL,
				SUMMON_BOMBS
			};

			bool aggroCondition(Creature* creature) override;
			bool deaggroCondition(Creature* creature) override;

			void aggroTransition() override;
			void deaggroTransition() override;

			void update_when_aggroed(Milliseconds elapsedTime) override;
			void update_when_deaggroed(Milliseconds elapsedTime) override;

			void deathTransition() override;

			Vector2d anchor_position; // boss always floats to it's initial spawn pos, being effectively stationary

			Timer summon_tentacle_cd;
			Timer summon_fireball_cd;
			Timer summon_bombs_cd;
		};
	}



	namespace item_entity {
		// # EldritchBattery #
		struct EldritchBattery : public m_type::ItemEntity {
			EldritchBattery() = delete;

			EldritchBattery(const Vector2d& position);
		};

		// # PowerShard #
		struct PowerShard : public m_type::ItemEntity {
			PowerShard() = delete;

			PowerShard(const Vector2d& position);
		};

		// # SpiderSignet #
		struct SpiderSignet : public m_type::ItemEntity {
			SpiderSignet() = delete;

			SpiderSignet(const Vector2d& position);
		};

		// # BoneMask #
		struct BoneMask : public m_type::ItemEntity {
			BoneMask() = delete;

			BoneMask(const Vector2d& position);
		};

		// # MagicNegator #
		struct MagicNegator : public m_type::ItemEntity {
			MagicNegator() = delete;

			MagicNegator(const Vector2d& position);
		};

		// # TwinSouls #
		struct TwinSouls : public m_type::ItemEntity {
			TwinSouls() = delete;

			TwinSouls(const Vector2d& position);
		};

		// # WatchingEye #
		struct WatchingEye : public m_type::ItemEntity {
			WatchingEye() = delete;

			WatchingEye(const Vector2d& position);
		};
	}



	namespace destructible {

		// # OrbOfBetrayal #
		// Deals damage to all creatures of it's own faction upon being destroyed
		// - Used during Mage bossfight
		class OrbOfBetrayal : public m_type::Destructible {
		public:
			OrbOfBetrayal() = delete;

			OrbOfBetrayal(const Vector2d &position);

		private:
			void effect() override;
		};
	}
}
