#include "entity_unique_s.h"

#include "game.h" /// playing sounds



using namespace ntt;
using namespace ntt::s;

/* ### projectile:: ### */

// # ArcaneProjectileBlue #
projectile::ArcaneProjectileBlue::ArcaneProjectileBlue(const Vector2d &position, const Vector2d &speed, const Damage &damage, double knockback, const Vector2d &AOE) :
	Projectile(position, damage, knockback, AOE)
{
	this->_init_sprite("[projectile]{arcane_projectile_blue}");

	constexpr auto HITBOX_SIZE = Vector2d(3., 3.);

	this->_init_solid(HITBOX_SIZE, speed);

	Game::ACCESS->play_sound("fire_cast.wav");
}

void projectile::ArcaneProjectileBlue::onCollision() {
	Projectile::onCollision();

	Game::ACCESS->play_sound("fire_impact.wav");
}



// # SpiritBomb #
projectile::SpiritBomb::SpiritBomb(const Vector2d& position, const Vector2d& speed, const Damage& damage, double knockback, const Vector2d& AOE) :
	Projectile(position, damage, knockback, AOE)
{
	this->_init_sprite("[projectile]{spirit_bomb}");

	constexpr auto HITBOX_SIZE = Vector2d(4., 4.);

	this->_init_solid(HITBOX_SIZE, speed);

	Game::ACCESS->play_sound("fire_cast.wav");
}

void projectile::SpiritBomb::onCollision() {
	Projectile::onCollision();

	Game::ACCESS->play_sound("fire_impact.wav");
}



/* ### particle:: ### */

// # OnDeathParticle #
namespace OnDeathParticle_consts {
	constexpr double MASS = 20;
	constexpr double FRICTION = 0.4;
}

particle::OnDeathParticle::OnDeathParticle(const Vector2d &position, const Vector2d &speed, const RGBColor &color, Milliseconds lifetime) :
	Particle(position, lifetime),
	color(color)
{
	// Init modules
	this->_init_sprite(false, "[particle]{on_death_particle}");

	this->sprite->color_mod = this->color;

	this->_optinit_solid(
		Vector2(3, 3),
		SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
		OnDeathParticle_consts::MASS,
		OnDeathParticle_consts::FRICTION
	);

	this->solid->speed = speed;
}