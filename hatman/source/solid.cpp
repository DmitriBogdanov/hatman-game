#include "solid.h"

#include "game.h" // access to timescale and game state
#include "globalconsts.hpp" // contains tile size (used in tile collision detection)
#include "tile_base.h" // 'Tile' type
#include "entity_base.h" // 'Entity' type



// # Entity::Physics #
SolidRectangle::SolidRectangle(Vector2d &parentPosition, const Vector2d &hitboxSize, SolidFlags flags, double mass = 1, double friction = 0) :
	parent_position(parentPosition),
	hitboxSize(hitboxSize),
	flags(flags),
	mass(mass),
	friction(friction),
	movement(0., 0.),
	speed(0., 0.),
	acceleration(0., 0.),
	enabled(true),
	is_grounded(false),
	is_dropping_down(false),
	total_force(0., 0.),
	friction_compensated(false)
{}

void SolidRectangle::update(Milliseconds elapsedTime) {
	if (!this->enabled) return;

	bool nonFrictionalHorizontalForcePresent = static_cast<bool>(this->total_force.x);

	// Apply gravity
	if (this->flags & SolidFlags::AFFECTED_BY_GRAVITY) { this->apply_GravityForce(); }

	// Apply friction
	if (this->is_grounded && !this->friction_compensated) { // handle with friction
		this->apply_FrictionForce();

		this->acceleration = this->total_force / this->mass;

		int oldSpeedXSign = helpers::sign(this->speed.x);
		this->speed += this->acceleration * ms_to_sec(elapsedTime);
		int newSpeedXSign = helpers::sign(this->speed.x);
		
		// If no external forces affect the solid, and speed.x changed its direction => friction stopped the body
		if (!nonFrictionalHorizontalForcePresent && oldSpeedXSign != newSpeedXSign) {
			this->speed.x = 0.;
		}
	}
	else { // handle without friction
		this->acceleration = this->total_force / this->mass;
		this->speed += this->acceleration * ms_to_sec(elapsedTime);
	}

	// Reset forces
	this->total_force = Vector2d();
	this->friction_compensated = false;

	// Calculate movement
	this->movement = this->speed * ms_to_sec(elapsedTime) + this->acceleration * ms_to_sec(elapsedTime) * ms_to_sec(elapsedTime) * 0.5;

	// Apply interaction with objects that may affect .movement
	if (this->flags & SolidFlags::SOLID) this->apply_TileCollisions();
	this->apply_LevelBorderCollisions();

	// Set final position
	this->parent_position += this->movement;
}

// Checks/getters
dRect SolidRectangle::getHitbox() const {
	return dRect(parent_position, this->hitboxSize, true);
}

Tile* SolidRectangle::getFirstCollision_Tile() const {
	const dRect entityRect = this->getHitbox();

	const Vector2 centerIndex = helpers::divide32(this->parent_position);
	const Vector2 adjustedHalfSize = Vector2(1, 1) + helpers::divide32(entityRect.getSize() * 0.5);
		// half-size of solid measured in tiles (rounded up), used to adjust collision checks for large solids

	const int leftBound = std::max(centerIndex.x - adjustedHalfSize.x, 0);
	const int rightBound = std::min(centerIndex.x + adjustedHalfSize.x, Game::READ->level->getSizeX() - 1);
	const int upperBound = std::max(centerIndex.y - adjustedHalfSize.y, 0);
	const int lowerBound = std::min(centerIndex.y + adjustedHalfSize.y, Game::READ->level->getSizeY() - 1);

	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tile = Game::ACCESS->level->getTile(X, Y);

			// Go over tile hitbox rects and check for collisions
			if (tile && tile->hitbox)
				for (const auto hitboxRect : tile->hitbox->rectangles)
					if (entityRect.overlapsWithRect(hitboxRect.rect))
						return tile;
		}

	return nullptr;
}

ntt::Entity* SolidRectangle::getFirstCollision_DifferentFactionEntity(Faction faction) const {
	const dRect entityRect = this->getHitbox();

	for (auto &otherEntity : Game::ACCESS->level->entities_killable)
		if (otherEntity->health->faction != faction && entityRect.overlapsWithRect(otherEntity->solid->getHitbox()))
			return otherEntity;
	
	return nullptr;
}

// Force
void SolidRectangle::applyForce(const Vector2d &force) {
	this->total_force += force;
}

void SolidRectangle::applyForce_Horizontal(double force) {
	this->total_force.x += force;
}

void SolidRectangle::applyForce_Left(double force) {
	this->total_force.x -= force;
}

void SolidRectangle::applyForce_Right(double force) {
	this->total_force.x += force;
}

void SolidRectangle::applyForce_Vertical(double force) {
	this->total_force.y += force;
}

void SolidRectangle::applyForce_Up(double force) {
	this->total_force.y -= force;
}

void SolidRectangle::applyForce_Down(double force) {
	this->total_force.y += force;
}

// Impulse
void SolidRectangle::addImpulse(const Vector2d &impulse) {
	this->speed += impulse / this->mass;
}

void SolidRectangle::addImpulse_Horizontal(double impulse) {
	this->speed.x += impulse / this->mass;
}

void SolidRectangle::addImpulse_Left(double impulse) {
	this->speed.x -= impulse / this->mass;
}

void SolidRectangle::addImpulse_Right(double impulse) {
	this->speed.x += impulse / this->mass;
}

void SolidRectangle::addImpulse_Vertical(double impulse) {
	this->speed.y += impulse / this->mass;
}

void SolidRectangle::addImpulse_Up(double impulse) {
	this->speed.y -= impulse / this->mass;
}

void SolidRectangle::addImpulse_Down(double impulse) {
	this->speed.y += impulse / this->mass;
}

// Additional movement methods (mostly used by creatures)
void SolidRectangle::applyFrictionCompensation() {
	this->friction_compensated = true;
}

void SolidRectangle::applyForceTillMaxSpeed_Left(double force, double maxSpeed) {
	if (this->speed.x < 0) this->applyFrictionCompensation();
		// no need to compensate friction if we accelerate in the opposite direction

	if (this->speed.x > -maxSpeed) this->applyForce_Left(force);
	else this->speed.x = -maxSpeed;
}

void SolidRectangle::applyForceTillMaxSpeed_Right(double force, double maxSpeed) {
	if (this->speed.x > 0) this->applyFrictionCompensation();
		// no need to compensate friction if we accelerate in the opposite direction

	if (this->speed.x < maxSpeed) this->applyForce_Right(force);
	else this->speed.x = maxSpeed;
}

void SolidRectangle::applyForceTillMaxSpeed_Horizontal(double force, double maxSpeed, Orientation orientation) {
	switch (orientation) {
	case Orientation::LEFT:
		this->applyForceTillMaxSpeed_Left(force, maxSpeed);
		break;

	case Orientation::RIGHT:
		this->applyForceTillMaxSpeed_Right(force, maxSpeed);
		break;

	default:
		break;
	}
}

void SolidRectangle::applyForceTillMaxSpeed_Up(double force, double maxSpeed) {
	if (this->speed.y > -maxSpeed) this->applyForce_Up(force);
	else this->speed.y = -maxSpeed;
}

void SolidRectangle::applyForceTillMaxSpeed_Down(double force, double maxSpeed) {
	if (this->speed.y < maxSpeed) this->applyForce_Down(force);
	else this->speed.y = maxSpeed;
}

void SolidRectangle::apply_GravityForce() {
	this->applyForce_Down(this->mass * physics::GRAVITY_ACCELERATION);
}

void SolidRectangle::apply_FrictionForce() {
	this->applyForce_Horizontal(
		-helpers::sign(this->speed.x) * this->mass * physics::GRAVITY_ACCELERATION * this->friction
	);	
}

void SolidRectangle::apply_TileCollisions() {
	this->is_grounded_at_left = false;
	this->is_grounded_at_right = false;

	dRect entityRect = this->getHitbox();
	entityRect.moveByX(this->movement.x);

	const Vector2 centerIndex = helpers::divide32(this->parent_position);
	const Vector2 adjustedHalfSize = Vector2(1, 1) + helpers::divide32(entityRect.getSize() * 0.5);
		// half-size of solid measured in tiles (rounded up), used to adjust collision checks for large solids

	// Collisions happens at RIGHT => iterate over 6 (or more for big solids) tiles at right
	if (this->movement.x > 0.) { 
		const int leftBound = std::max(centerIndex.x, 0); // no need to check left column
		const int rightBound = std::min(centerIndex.x + adjustedHalfSize.x, Game::READ->level->getSizeX() - 1);
		const int upperBound = std::max(centerIndex.y - adjustedHalfSize.y, 0);
		const int lowerBound = std::min(centerIndex.y + adjustedHalfSize.y, Game::READ->level->getSizeY() - 1);

		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				// Go over tile hitbox rects and check for collisions
				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles)
						if (entityRect.overlapsWithRect(hitboxRect.rect) && !hitboxRect.is_platform) {
							entityRect.moveRightTo(hitboxRect.rect.getLeft());
							this->speed.x = 0.;
						}
			}
	}
	// Collisions happens at LEFT => iterate over 6 (or more for big solids) tiles at left
	else if (this->movement.x < 0.) {
		const int leftBound = std::max(centerIndex.x - adjustedHalfSize.x, 0);
		const int rightBound = std::min(centerIndex.x, Game::READ->level->getSizeX() - 1); // no need to check right column
		const int upperBound = std::max(centerIndex.y - adjustedHalfSize.y, 0);
		const int lowerBound = std::min(centerIndex.y + adjustedHalfSize.y, Game::READ->level->getSizeY() - 1);

		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				// Go over tile hitbox rects and check for collisions
				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles)
						if (entityRect.overlapsWithRect(hitboxRect.rect) && !hitboxRect.is_platform) {
							entityRect.moveLeftTo(hitboxRect.rect.getRight());
							this->speed.x = 0.;
						}
			}
	}

	entityRect.moveByY(this->movement.y);

	// Collisions happens at BOTTOM => iterate over 6 (or more for big solids) tiles at bottom
	if (this->movement.y > 0.) {
		const int leftBound = std::max(centerIndex.x - adjustedHalfSize.x, 0);
		const int rightBound = std::min(centerIndex.x + adjustedHalfSize.x, Game::READ->level->getSizeX() - 1);
		const int upperBound = std::max(centerIndex.y, 0); // no need to check upper row
		const int lowerBound = std::min(centerIndex.y + adjustedHalfSize.y, Game::READ->level->getSizeY() - 1);
		///const int lowerBound = std::min(centerIndex.y + performance::COLLISION_CHECK_DEPH, Game::READ->level->getSizeY() - 1);

		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				// Go over tile hitbox rects and check for collisions
				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles) {
						// Handle collision with rect
						if (entityRect.overlapsWithRect(hitboxRect.rect)) {
							// regular collision
							if (!hitboxRect.is_platform) { 
								entityRect.moveBottomTo(hitboxRect.rect.getTop());
								this->speed.y = 0.;
								this->is_grounded = true;
							}
                            // collision with platform, condition may need fine-tuning
							else if (entityRect.getBottom() < hitboxRect.rect.getTop() + physics::PLATFORM_EPSILON && !this->is_dropping_down) {
								entityRect.moveBottomTo(hitboxRect.rect.getTop());
								this->speed.y = 0.;
								this->is_grounded = true;
							}
                        }

						// Deduce whether left/right sides are grounded
						if (this->is_grounded) {
							if (hitboxRect.rect.getLeft() < entityRect.getLeft())
								this->is_grounded_at_left = true;

							if (hitboxRect.rect.getRight() > entityRect.getRight())
								this->is_grounded_at_right = true;
								// logic can be simplified to this since we already know that collision happened
								// and it happened precisely at the bottom
						}
					}
			}
	}
	// Collisions happens at TOP => iterate over 6 (or more for big solids) tiles at top
	else if (this->movement.y < 0.) {
		const int leftBound = std::max(centerIndex.x - adjustedHalfSize.x, 0);
		const int rightBound = std::min(centerIndex.x + adjustedHalfSize.x, Game::READ->level->getSizeX() - 1);
		const int upperBound = std::max(centerIndex.y - adjustedHalfSize.y, 0);
		const int lowerBound = std::min(centerIndex.y, Game::READ->level->getSizeY() - 1); // no need to check lower row

		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				// Go over tile hitbox rects and check for collisions
				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles)
						if (!hitboxRect.is_platform && entityRect.overlapsWithRect(hitboxRect.rect)) {
							entityRect.moveTopTo(hitboxRect.rect.getBottom());
							this->speed.y = 0.;
						}
			}
	}

	this->movement = entityRect.getCenter() - this->parent_position;
}

void SolidRectangle::apply_LevelBorderCollisions() {
	const dRect entityHitbox = this->getHitbox();
	const double levelWidth = Game::READ->level->getSize().x * natural::TILE_SIZE; // Assumes rendering size is equal to physica;
	//const double levelHeight = Game::READ->level->getSize().y * natural::TILE_SIZE;

	/// TEMP, REMOVE LATER
	/*if (entityHitbox.getBottom() > levelHeight) {
		parent_position.y = levelHeight - entityHitbox.getSizeY() / 2.;
		this->speed.y = 0.;
		this->is_grounded = true;
	}
	else if (entityHitbox.getTop() < 0) {
		parent_position.y = entityHitbox.getSizeY() / 2.;
		this->speed.y = 0.;
	}*/
	/// ^ TEMP

	if (entityHitbox.getRight() > levelWidth) {
		parent_position.x = levelWidth - entityHitbox.getSizeX() / 2.;
		this->speed.x = 0.;
	}
	else if (entityHitbox.getLeft() < 0) {
		parent_position.x = entityHitbox.getSizeX() / 2.;
		this->speed.x = 0.;
	}
}