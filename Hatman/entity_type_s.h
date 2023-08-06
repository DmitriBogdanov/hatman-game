#pragma once

/*
Suffix _s stands for 'spawnable' entities that:
- Take multiple parameters in a contructor that determine ther specific values
- Can't be parsed from a map file
- Are only spawned during the gameplay
*/

#include "entity_base.h"



// ntt::s_type::
// - 's' stand for 'spawnable' - aka entities CAN'T be parsed from a map file
// - Contains all types of 's' entities
namespace ntt::s_type {
	// # Projectile #
	// - Deals damage to all creatures of different faction
	// - Can fly in arch or a straight line
	class Projectile : public Entity {
	public:
		Projectile() = delete;

		Projectile(const Vector2d &position, const Damage &damage, double knockback, const Vector2d &AOE);

		virtual ~Projectile() = default;

		TypeId type_id() const override;

		bool update(Milliseconds elapsedTime) override;

		Timer delay; // stops projectile logic when this timer is set

	protected:
		Damage damage;
		double knockback;
		Vector2d AOE; // size of of area where damage and knockbacks is applied upon 'explosion'

		Timer lifetime; // prevents prokectiles from persisting indefinetely offscreen

		// Checks
		bool checkEntityCollision(); // ignores entities of the same faction
		bool checkTerrainCollision();

		// Effects
		virtual void onCollision(); // projectile 'explodes' dealing .damage to all enemies in its hitbox

		// Module inits
		void _init_sprite(const std::string &folder); // inits with 'default' and 'explosion' animasions
		void _init_solid(const Vector2d &hitboxSize, const Vector2d &speed, bool affectedByGravity = false, bool collidesWithTerrain = true);

	private:
		bool collides_with_terrain;

		ControllableSprite* _sprite; // casted version of 'sprite', used to access methods in 'ControllableSprite'
	};



	// # Particle #
	// - Uses static OR animated sprite
	// - Solid is optional
	// - Has no health
	// - Has a lifetime (limited or not), erases itself at the end of it
	class Particle : public Entity {
	public:
		Particle() = delete;

		Particle(const Vector2d &position); // creates particle with infinite lifetime
		Particle(const Vector2d &position, Milliseconds lifetime); // creates particle with finite lifetime

		virtual ~Particle() = default;

		TypeId type_id() const override;

		virtual bool update(Milliseconds elapsedTime) override;

	protected:
		void _init_sprite(bool animated, const std::string &folder, const std::string &filename = DEFAULT_ANIMATION_NAME);
		virtual void _optinit_solid(const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction);
		// optional, inits solid

	private:
		bool lifetime_is_limited;
		Milliseconds lifetime_left;
	};
}