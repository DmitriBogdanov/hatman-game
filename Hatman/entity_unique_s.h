#pragma once

/*
Suffix _s stands for 'spawnable' entities that:
- Take multiple parameters in a contructor that determine ther specific values
- Can't be parsed from a map file
- Are only spawned during the gameplay
*/

#include "entity_type_s.h" // entity_type_s:: base classes



// ntt::s::
// - 's' stand for 'spawnable' - aka entities CAN'T be parsed from a map file
// - Contains all unique 's' entities
namespace ntt::s {
	namespace projectile {
		// # ArcaneProjectileBlue #
		// - Fast projectile flying in a straight line
		// - Explodes upon impact
		class ArcaneProjectileBlue : public s_type::Projectile {
		public:
			ArcaneProjectileBlue() = delete;

			ArcaneProjectileBlue(const Vector2d &position, const Vector2d &speed, const Damage &damage, double knockback, const Vector2d &AOE);
		
		private:
			void onCollision() override;
		};

		// # SpiritBomb #
		// - Arching projectile with big AOE
		// - Used by Spirit bombers
		class SpiritBomb : public s_type::Projectile {
		public:
			SpiritBomb() = delete;

			SpiritBomb(const Vector2d& position, const Vector2d& speed, const Damage& damage, double knockback, const Vector2d& AOE);
		
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