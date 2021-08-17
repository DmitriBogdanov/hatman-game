#include "entity_type_s.h"

#include "graphics.h" // access to texture loading
#include "game.h" // access to game state



using namespace ntt;

// # Projectile #
namespace Projectile_consts {
	constexpr double DEFAULT_MASS = 30.;
	constexpr double DEFAULT_FRICTION = 0.5;
}

s_type::Projectile::Projectile(const Vector2d &position, const Damage &damage, double knockback, const Vector2d &AOE) :
	Entity(position),
	damage(damage),
	knockback(knockback),
	AOE(AOE)
{}

TypeId s_type::Projectile::type_id() const { return TypeId::PROJECTILE; }

bool s_type::Projectile::update(Milliseconds elapsedTile) {
	if (!Entity::update(elapsedTile)) return false;

	this->sprite->flip = (this->solid->speed.x >= 0.) ? SDL_FLIP_NONE : SDL_FLIP_VERTICAL;
	this->sprite->setRotation(this->solid->speed.angleToX());

	if (this->solid->enabled && (this->checkEntityCollision() || this->checkTerrainCollision())) this->onCollision();

	return true;
}

// Checks
bool s_type::Projectile::checkEntityCollision() {
	return this->solid->getFirstCollision_DifferentFactionEntity(this->damage.faction);
}

bool s_type::Projectile::checkTerrainCollision() {
	return this->collides_with_terrain && this->solid->getFirstCollision_Tile();
}

// Effects
void s_type::Projectile::onCollision() {
	// Stop moving and play explosion animation
	this->solid->enabled = false;

	const auto area = dRect(this->position, this->AOE, true);

	// Look for entities that should be damaged, deal damage and knockback
	for (auto &entity : Game::ACCESS->level->entities_killable)
		if (area.overlapsWithRect(entity->solid->getHitbox())) {
			entity->health->applyDamage(this->damage);

			const auto knockbackDirection = (entity->position - this->position).normalized();
			entity->solid->addImpulse(knockbackDirection * this->knockback);
		}

	this->_sprite->animation_play("explosion");
	this->mark_for_erase(this->_sprite->animation_duration("explosion"));
}

// Module inits
void s_type::Projectile::_init_sprite(const std::string &folder) {
	this->_parse_controllable_sprite(folder, { DEFAULT_ANIMATION_NAME, "explosion" });

	this->_sprite = static_cast<ControllableSprite*>(this->sprite.get());
}

void s_type::Projectile::_init_solid(const Vector2d &hitboxSize, const Vector2d &speed, bool affectedByGravity, bool collidesWithTerrain) {
	const auto flags = affectedByGravity ? SolidFlags::AFFECTED_BY_GRAVITY : SolidFlags::NONE;

	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		flags,
		Projectile_consts::DEFAULT_MASS,
		Projectile_consts::DEFAULT_FRICTION
		);

	this->solid->speed = speed;

	this->collides_with_terrain = collidesWithTerrain;
}



// # Particle #
s_type::Particle::Particle(const Vector2d &position) :
	Entity(position),
	lifetime_is_limited(false),
	lifetime_left(0.)
{}

s_type::Particle::Particle(const Vector2d &position, Milliseconds lifetime) :
	Entity(position),
	lifetime_is_limited(true),
	lifetime_left(lifetime)
{}

TypeId s_type::Particle::type_id() const { return TypeId::PARTICLE; }

bool s_type::Particle::update(Milliseconds elapsedTime) {
	if (!Entity::update(elapsedTime)) return false;

	if (this->lifetime_is_limited) {
		this->lifetime_left -= elapsedTime;

		if (this->lifetime_left < 0.) this->mark_for_erase();
	}

	return true;
}

// Module inits
void s_type::Particle::_init_sprite(bool animated, const std::string &folder, const std::string &filename) {
	if (animated)
		this->_parse_animated_sprite(folder, filename);
	else
		this->_parse_static_sprite(folder, filename);
}

void s_type::Particle::_optinit_solid(const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction) {
	this->solid = std::make_unique<SolidRectangle>(
		this->position,
		hitboxSize,
		flags,
		mass,
		friction
		);
}