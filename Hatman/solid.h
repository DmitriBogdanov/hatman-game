#pragma once

#include <unordered_set> // related type

#include "timer.h" // 'Milliseconds' type
#include "geometry_utils.h" // geometry types
#include "stats.h" // 'Faction' enum



// # SolidFlags #
// - Enum of all flags that apply to solids and determine their behaviour
enum class SolidFlags {
	NONE = 0,
	SOLID = 1 << 0,
	AFFECTED_BY_GRAVITY = 1 << 1
};

constexpr SolidFlags operator | (SolidFlags flag1, SolidFlags flag2) {
	return static_cast<SolidFlags>(static_cast<int>(flag1) | static_cast<int>(flag2));
}

constexpr int operator & (SolidFlags flag1, SolidFlags flag2) {
	return static_cast<int>(flag1) & static_cast<int>(flag2);
		// returning int so we don't have to cast inside each 'if' statement
		// we can do this because solid flags aren't really &'ed in any context outside conditions
}



class Tile; // we need pointers to these
namespace ntt { class Entity; }

// # SolidRectangle #
// - Represents a rectangle with physics attached to it
// - Behaviour depends on active flags
class SolidRectangle {
public:
	SolidRectangle() = delete;

	SolidRectangle(Vector2d &parentPosition, const Vector2d &hitboxSize, SolidFlags flags, double mass, double friction);

	void update(Milliseconds elapsedTime);

	// Properties
	Vector2d &parent_position; // position of the object module is attached to
	Vector2d hitboxSize; // hitbox rectangle with a center in parent_position

	SolidFlags flags;

	double mass;
	double friction; // slows down grounded objects horizontaly by its value per second

	Vector2d movement; // position change before applying collisions
	Vector2d speed;
	Vector2d acceleration; // acceleration not accounting for constants aka gravity

	// State
	bool enabled; // stops physics when false

	bool is_grounded;

	bool is_dropping_down; // fall through platforms when true

	// Checks/getters
	dRect getHitbox() const;
	Tile* getFirstCollision_Tile() const; // nullptr if no tile collisions were found
	ntt::Entity* getFirstCollision_DifferentFactionEntity(Faction faction) const; // ignores collisions with entities who have no .health or .solid

	// Force
	void applyForce(const Vector2d &force);
	void applyForce_Horizontal(double force); // sign(force) dictates direction
	void applyForce_Left(double force);
	void applyForce_Right(double force);
	void applyForce_Vertical(double force); // sign(force) dictates direction
	void applyForce_Up(double force);
	void applyForce_Down(double force);

	// Impulse
	void addImpulse(const Vector2d &impulse);
	void addImpulse_Horizontal(double impulse); // sign(impulse) dictates direction
	void addImpulse_Left(double impulse);
	void addImpulse_Right(double impulse);
	void addImpulse_Vertical(double impulse); // sign(impulse) dictates direction
	void addImpulse_Up(double impulse);
	void addImpulse_Down(double impulse);

	// Additional movement methods (mostly used by creatures)
	void applyFrictionCompensation(); // applies force equal to friction in the opposite direction

	void applyForceTillMaxSpeed_Left(double force, double maxSpeed); // compensates friction and applies force untill max speed is rached
	void applyForceTillMaxSpeed_Right(double force, double maxSpeed);
	void applyForceTillMaxSpeed_Horizontal(double force, double maxSpeed, Orientation orientation);
	
private:
	Vector2d total_force;
	bool friction_compensated; // when true friction is not applied
	
	void apply_GravityForce(); // applies gravity as a downwards force
	void apply_FrictionForce(); // all grounded object experience friction force
	void apply_TileCollisions();
	void apply_LevelBorderCollisions();
};