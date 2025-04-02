#pragma once

/*
Suffix _s stands for 'spawnable' entities that:
- Take multiple parameters in a constructor that determine ther specific values
- Can't be parsed from a map file
- Are only spawned during the gameplay
*/

#include "entity/type_s.h" // entity_type_s:: base classes



// ntt::s::
// - 's' stand for 'spawnable' - aka entities CAN'T be parsed from a map file
// - Contains all unique 's' entities
namespace ntt::s {
	namespace projectile {
		// # SpiritBomb #
		// - Moderately fast projectile with AOE explosion
		// - Stopped by terrain
		// - Used by "SpiritBomber"
		class SpiritBomb : public s_type::Projectile {
		public:
			SpiritBomb() = delete;

			SpiritBomb(const Vector2d& position, const Vector2d& speed, const Damage& damage, double knockback, const Vector2d& AOE);
		
		private:
			void onCollision() override;
		};

		// # Fireball #
		// - Slowly moving projectile that deals damage on contact
		// - Passes through terrain
		// - Used by "CultistMage" and bosses
		class Fireball : public s_type::Projectile {
		public:
			Fireball() = delete;

			Fireball(const Vector2d& position, const Vector2d& speed, const Damage& damage, double knockback, const Vector2d& AOE);

		private:
			void onCollision() override;
		};
	}



	namespace particle {
		// # OnDeathParticle #
		// - Single pixel particle
		class OnDeathParticle : public s_type::Particle {
		public:
			OnDeathParticle() = delete;

			OnDeathParticle(const Vector2d &position, const Vector2d &speed, const RGBColor &color, Milliseconds lifetime);

		private:
			RGBColor color;
		};
	}
}