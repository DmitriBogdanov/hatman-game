#pragma once

/* Contains module: 'Health' */

#include <SDL.h> // 'SDL_Texture*' type for 'HealthbarDisplay'
#include "geometry_utils.h" // 'Vector2d' and 'srcRect' types for 'HealthbarDisplay'
#include "timer.h" // 'Milliseconds' type for updating



using uint = unsigned int; // typedefs for convenient future refactioring
using sint = int;



// # Faction #
// - Enum that represents entity faction
// - Creatures can't damage their own faction
enum class Faction {
	PLAYER,
	PROPS,
	SHADOW
};



// # Damage #
// - Represents a damage packet
struct Damage {
	Damage() = default;

	constexpr Damage(Faction faction, double phys = 0, double magic = 0, double dot = 0, double pure = 0) :
		phys(phys),
		magic(magic),
		dot(dot),
		pure(pure),
		faction(faction)
	{}

	constexpr Damage& operator+=(const Damage& other) {
		this->phys += other.phys;
		this->magic += other.magic;
		this->dot += other.dot;
		this->pure += other.pure;

		return *this;
	}

	constexpr Damage operator+(const Damage& other) {
		return Damage(
			this->faction,
			this->phys + other.phys,
			this->magic + other.magic,
			this->dot + other.dot,
			this->pure + other.pure
		);
	}

	constexpr Damage& operator*=(double modifier) {
		this->phys *= modifier;
		this->magic *= modifier;
		this->dot *= modifier;
		this->pure *= modifier;

		return *this;
	}

	constexpr Damage operator*(double modifier) {
		return Damage(
			this->faction,
			this->phys * modifier,
			this->magic * modifier,
			this->dot * modifier,
			this->pure * modifier
		);
	}

	constexpr Damage& modify(double modifierPhys, double modifierMagic, double modifierDot, double modifierPure) {
		this->phys *= modifierPhys;
		this->magic *= modifierMagic;
		this->dot *= modifierDot;
		this->pure *= modifierPure;

		return *this;
	}

	double phys;
	double magic;
	double dot;
	double pure;

	Faction faction;
};



// # Health #
// - MODULE
// - Containt health and resists with all their modifiers
// - Calculated and applies damage taken
class Health {
public:
	Health() = delete;

	Health(Faction faction, uint maxHp, sint regen = 0, sint physRes = 0, sint magicRes = 0, sint dotRes = 0); // sets base stats

	void update(Milliseconds elapsedTime);

	void setFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes);
	void addFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint dotRes);

	void setMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes);
	void addMulti(double maxHp, double regen, double physRes, double magicRes, double dotRes);

	void applyDamage(const Damage &damage);
	void applyHeal(double heal);

	void instakill();

	// Getters
	bool dead() const; // true if hp < 0
	double percentage() const; // returns values from 0.0 to 1.0

	double hp; // current hp

	Faction faction; // health receives no damage from the same faction
private:
	// Base values
	uint base_maxHp;
	sint base_regen;
	sint base_physRes;
	sint base_magicRes;
	sint base_dotRes;

	// Flat mods
	uint flat_maxHp = 0;
	sint flat_regen = 0;
	sint flat_physRes = 0;
	sint flat_magicRes = 0;
	sint flat_dotRes = 0;

	// Multiplicative mods
	double multi_maxHp = 0;
	double multi_regen = 0;
	double multi_physRes = 0;
	double multi_magicRes = 0;
	double multi_dotRes = 0;

	// Total values
	uint total_maxHp;
	sint total_regen;
	sint total_physRes;
	sint total_magicRes;
	sint total_dotRes;

	void recalc(); // recalculates total values
};



// # HealthbarDisplay #
// - Connects to some position and a 'Health' object, displays current health
// - No need to update, healthbar merely displays state of other objects
class HealthbarDisplay {
public:
	HealthbarDisplay() = delete;
	HealthbarDisplay(const Vector2d &parentPosition, const Health &parentHealth, const Vector2d &bottomCenterpointAlignment);
		// note that for convenience constructor takes alignment of the bottom centerpoint and THE calculates actual corner alignment

	void draw() const;
private:
	const Vector2d &parent_position;
	const Health &parent_health;

	Vector2d corner_alignment; // alignment relative to the .parent_position

	SDL_Texture* texture;
};