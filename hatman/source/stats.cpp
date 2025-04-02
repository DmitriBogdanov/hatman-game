#include "stats.h"

#include "graphics.h" // access to texture loading and rendering to camera
#include "globalconsts.hpp" // screen size for GUI



// # Health #
const sint MAX_RESISTANCE = 90;

Health::Health(Faction faction, uint maxHp, sint regen, sint physRes, sint magicRes, sint chaosRes) :
	faction(faction),
    time_since_last_damage_received(0),
	base_maxHp(maxHp),
	base_regen(regen),
	base_physRes(physRes),
	base_magicRes(magicRes),
	base_chaosRes(chaosRes)
{
	this->recalc();

	this->hp = this->total_maxHp;
}

void Health::update(Milliseconds elapsedTime) {
	this->applyHeal(static_cast<double>(total_regen) / 1000. * elapsedTime); // account for time elapsed

	this->time_since_last_damage_received += elapsedTime;
}

void Health::setFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint chaosRes) {
	this->flat_maxHp = maxHp;
	this->flat_regen = regen;
	this->flat_physRes = physRes;
	this->flat_magicRes = magicRes;
	this->flat_chaosRes = chaosRes;

	this->recalc();
}
void Health::addFlat(uint maxHp, sint regen, sint physRes, sint magicRes, sint chaosRes) {
	this->flat_maxHp += maxHp;
	this->flat_regen += regen;
	this->flat_physRes += physRes;
	this->flat_magicRes += magicRes;
	this->flat_chaosRes += chaosRes;

	this->recalc();
}

void Health::setMulti(double maxHp, double regen, double physRes, double magicRes, double chaosRes) {
	this->multi_maxHp = maxHp;
	this->multi_regen = regen;
	this->multi_physRes = physRes;
	this->multi_magicRes = magicRes;
	this->multi_chaosRes = chaosRes;

	this->recalc();
}
void Health::addMulti(double maxHp, double regen, double physRes, double magicRes, double chaosRes) {
	this->multi_maxHp += maxHp;
	this->multi_regen += regen;
	this->multi_physRes += physRes;
	this->multi_magicRes += magicRes;
	this->multi_chaosRes += chaosRes;

	this->recalc();
}

void Health::recalc() {
	const double currentHpPercentage = this->hp / this->total_maxHp;

	this->total_maxHp = static_cast<uint>((this->base_maxHp + this->flat_maxHp) * (1. + this->multi_maxHp));
	this->total_regen = static_cast<sint>((this->base_regen + this->flat_regen) * (1. + this->multi_regen));
	this->total_physRes = static_cast<sint>((this->base_physRes + this->flat_physRes) * (1. + this->multi_physRes));
	this->total_magicRes = static_cast<sint>((this->base_magicRes + this->flat_magicRes) * (1. + this->multi_magicRes));
	this->total_chaosRes = static_cast<sint>((this->base_chaosRes + this->flat_chaosRes) * (1. + this->multi_chaosRes));

	this->hp = this->total_maxHp * currentHpPercentage;

	// Cap resistances
	if (this->total_physRes > MAX_RESISTANCE) { this->total_physRes = MAX_RESISTANCE; }
	if (this->total_magicRes > MAX_RESISTANCE) { this->total_magicRes = MAX_RESISTANCE; }
	if (this->total_chaosRes > MAX_RESISTANCE) { this->total_chaosRes = MAX_RESISTANCE; }
}

void Health::applyDamage(const Damage &damage) {
	if (this->faction != damage.faction) {
		this->hp -= damage.phys * (1. - static_cast<double>(total_physRes) / 100.); // phys part
		this->hp -= damage.magic * (1. - static_cast<double>(total_magicRes) / 100.); // magic part
		this->hp -= damage.chaos * (1. - static_cast<double>(total_chaosRes) / 100.); // chaos part
		this->hp -= damage.pure; // pure part

		this->time_since_last_damage_received = 0;
	}
}

void Health::applyHeal(double heal) {
	if (this->hp < this->total_maxHp) {
		this->hp += heal;

		// Doesn't let hp overcap
		if (this->hp > this->total_maxHp) {
			this->hp = this->total_maxHp;
		}
	}
}

void Health::instakill() {
	this->hp = -666666;
	time_since_last_damage_received = 0;
}

bool Health::dead() const {
	return (this->hp <= 0.);
}
double Health::percentage() const {
	return this->hp / this->total_maxHp;
}



// # HealthbarDisplay #
namespace HealthbarDisplay_consts {
	constexpr auto HEALTHBAR_SIZE = Vector2(20, 4);
}

HealthbarDisplay::HealthbarDisplay(const Vector2d &parentPosition, const Health &parentHealth, const Vector2d &bottomCenterpointAlignment) :
	parent_position(parentPosition),
	parent_health(parentHealth),
	corner_alignment(bottomCenterpointAlignment - Vector2d(HealthbarDisplay_consts::HEALTHBAR_SIZE.x / 2., HealthbarDisplay_consts::HEALTHBAR_SIZE.y))
{
	// Load texture
	this->sprite.setTexture(Graphics::ACCESS->getTexture_GUI("healthbar.png"));
}

void HealthbarDisplay::draw() {
	const double percentage = this->parent_health.percentage();


	// Draw healthbar fill
	this->sprite.setTextureRect(sf::IntRect(
		1,
		1,
		static_cast<int>(18. * percentage),
		2
	));

	this->sprite.setPosition(
		static_cast<float>(this->parent_position.x + this->corner_alignment.x + 1.),
		static_cast<float>(this->parent_position.y + this->corner_alignment.y + 1.)
	);

	// > no scaling needed

	Graphics::ACCESS->camera->draw_sprite(this->sprite);

	// Draw healthbar frame
	this->sprite.setTextureRect(sf::IntRect(
		0,
		4,
		20,
		4
	));

	this->sprite.setPosition(
		static_cast<float>(this->parent_position.x + this->corner_alignment.x),
		static_cast<float>(this->parent_position.y + this->corner_alignment.y)
	);

	// > no scaling needed

	Graphics::ACCESS->camera->draw_sprite(this->sprite);
}



// # BossHealthbarDisplay #
namespace BossHealthbarDisplay_consts {
	// Fill
	constexpr Vector2 FILL_CORNER = Vector2(2, 2);
	constexpr Vector2 FILL_SIZE = Vector2(376, 6);

	// Frame
	constexpr Vector2 FRAME_CORNER = Vector2(0, 10);
	constexpr Vector2 FRAME_SIZE = Vector2(380, 10);

	// Healthbar alignment
	constexpr int DISTANCE_FROM_BOTTOM = 32;

	constexpr Vector2 BOTTOM_CENTERPOINT = Vector2(static_cast<int>(natural::WIDTH / 2.), natural::HEIGHT - DISTANCE_FROM_BOTTOM);
	constexpr Vector2 CORNER_ALIGNMENT = BOTTOM_CENTERPOINT - FRAME_SIZE / 2.;

	// Boss title
	constexpr int DISTANCE_FROM_HEALTHBAR = 12;

	constexpr Vector2d TEXT_CENTERPOINT_ALIGNMENT = Vector2d(BOTTOM_CENTERPOINT.x, BOTTOM_CENTERPOINT.y - FRAME_SIZE.y - DISTANCE_FROM_HEALTHBAR);

	constexpr double TEXT_SCALE = 2;
}

BossHealthbarDisplay::BossHealthbarDisplay(const Health &parentHealth, const std::string &bossTitle) :
	parent_health(parentHealth),
	boss_title(bossTitle)
{
	// Load texture
	this->sprite.setTexture(Graphics::ACCESS->getTexture_GUI("boss_healthbar.png"));
}

void BossHealthbarDisplay::draw() {
	using namespace BossHealthbarDisplay_consts;

	Font* const font = Graphics::READ->gui->fonts.at("BLOCKY").get();

	const double percentage = this->parent_health.percentage();

	// Draw healthbar fill
	this->sprite.setTextureRect(sf::IntRect(
		FILL_CORNER.x,
		FILL_CORNER.y,
		static_cast<int>(FILL_SIZE.x * percentage),
		FILL_SIZE.y
	));

	this->sprite.setPosition(
		static_cast<float>(CORNER_ALIGNMENT.x + FILL_CORNER.x),
		static_cast<float>(CORNER_ALIGNMENT.y + FILL_CORNER.y)
	);

	Graphics::ACCESS->gui->draw_sprite(this->sprite);

	// Draw healthbar frame
	this->sprite.setTextureRect(sf::IntRect(
		FRAME_CORNER.x,
		FRAME_CORNER.y,
		FRAME_SIZE.x,
		FRAME_SIZE.y
	));

	this->sprite.setPosition(
		static_cast<float>(CORNER_ALIGNMENT.x),
		static_cast<float>(CORNER_ALIGNMENT.y)
	);

	Graphics::ACCESS->gui->draw_sprite(this->sprite);

	// Draw title
	font->color_set(colors::SH_YELLOW);
	font->scale_set(TEXT_SCALE);
	font->draw_line_centered(TEXT_CENTERPOINT_ALIGNMENT, this->boss_title);
	font->scale_reset();
}