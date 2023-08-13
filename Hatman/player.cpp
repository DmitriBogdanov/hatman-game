#include "player.h"

#include "filepaths.hpp" // path to textures
#include "graphics.h" // access to texture loading
#include "game.h" // access to inputs
#include "controls.h" // access to control keys
#include "globalconsts.hpp" // physical consts
#include "entity_unique_s.h" // for spawning projectile entities
#include "ct_math.hpp" // for calculating jump speed



using namespace ntt;
using namespace ntt::player;

// # Player #
namespace Player_consts {
	constexpr auto HITBOX_SIZE = Vector2d(10., 30.);

	constexpr double MASS = 100.;
	constexpr double FRICTION = 0.6;

	constexpr double MOVEMENT_SPEED = 90.;
	constexpr double MOVEMENT_ACCELERATION = 600.;
	constexpr double MOVEMENT_FORCE = MASS * MOVEMENT_ACCELERATION;

	constexpr double JUMP_SPEED = ct_speed_corresponding_to_jump_height(physics::GRAVITY_ACCELERATION, 36.);
	constexpr double JUMP_IMPULSE = MASS * JUMP_SPEED;

	constexpr Milliseconds DROPPING_DOWN_STICKY_DELAY_DURATION = 
		sec_to_ms(ct_sqrt(2 * physics::PLATFORM_EPSILON / physics::GRAVITY_ACCELERATION) + 1e-4);
		// required time is chosen based on guaranteeing that player falls down further than platform epsilon
		// g t^2 / 2 = platform_epsion   =>   t = sqrt(2 * platform_epsion / g)

	constexpr uint BASE_HP = 1000;
	constexpr sint BASE_REGEN = 25;
	constexpr sint BASE_PHYS_RES = 0;
	constexpr sint BASE_MAGIC_RES = 0;
	constexpr sint BASE_CHAOS_RES = 0;

	constexpr Milliseconds OUT_OF_COMBAT_REGEN_TIMER = sec_to_ms(10.);
	constexpr double OUT_OF_COMBAT_REGEN_MULTI = 8.;
	
	constexpr double STAND_TO_MOVE_ANIMATION_SPEEDUP = 3.;
	constexpr double STAND_TO_ATTACK_ANIMATION_SPEEDUP = 3.;
	
	// Charges
	constexpr uint BASE_CHARGES = 3;
	constexpr uint MAX_CHARGES_CAP = 5;

	constexpr uint SKILL_CHARGE_COST = 2;
	constexpr uint JUMP_CHARGE_COST = 1;

	constexpr Milliseconds CHARGE_CD = sec_to_ms(2);

	// Death
	constexpr int PARTICLE_COUNT = 16;
	constexpr double PARTICLE_MAX_SPEED_X = 200.;
	constexpr double PARTICLE_MAX_SPEED_Y = 250.;
	constexpr Milliseconds PARTICLE_DURATION_MIN = sec_to_ms(1.);
	constexpr Milliseconds PARTICLE_DURATION_MAX = sec_to_ms(6.);
}

namespace camera {
	constexpr double TRAP_SIZE_LEFT = 15.;
	constexpr double TRAP_SIZE_RIGHT = TRAP_SIZE_LEFT;
	constexpr double TRAP_SIZE_UP = 5.;
	constexpr double TRAP_SIZE_DOWN = 40.;
		// camera moves in a way the player stays inside 'camera trap' rectangle
		// gives camera smoother feel

	constexpr double SPEED_COEF_X = 20.;
	constexpr double SPEED_COEF_Y = SPEED_COEF_X;

	constexpr double EPSILON_X = 5.;
	constexpr double EPSILON_Y = 5.;
		// camera doesn't go slower if distance is less than epsilon
		// prevents camera getting ever-assyptotically-closer, makes it go no less than certain speed
}

namespace fire {
	// Chain
	constexpr double CHAIN_RANGE_FRONT = 30.;
	constexpr double CHAIN_RANGE_BACK = 2.;
	constexpr double CHAIN_RANGE_UP = 20.;
	constexpr double CHAIN_RANGE_DOWN = 12.;

	const auto CHAIN_0_DAMAGE = Damage(Faction::PLAYER, 0., 300., 0., 50.);
	constexpr double CHAIN_0_KNOCKBACK_X = 20. * 100.;
	constexpr double CHAIN_0_KNOCKBACK_Y = 0. * 100.;

	const auto CHAIN_2_DAMAGE = Damage(Faction::PLAYER, 0., 400., 0., 50.);
	constexpr double CHAIN_2_KNOCKBACK_X = 120. * 100.;
	constexpr double CHAIN_2_KNOCKBACK_Y = 110. * 100.;

	// Ult
	constexpr double ULT_TELEPORT_RANGE = 50.;

	constexpr double ULT_RANGE_FRONT = 40.;
	constexpr double ULT_RANGE_BACK = 2.;
	constexpr double ULT_RANGE_UP = 20.;
	constexpr double ULT_RANGE_DOWN = 12.;

	const auto ULT_DAMAGE = Damage(Faction::PLAYER, 0., 750., 0., 50.);

	constexpr double ULT_KNOCKBACK_X = 100. * 100.;
	constexpr double ULT_KNOCKBACK_Y = 200. * 100.;
}



Player::Player(const Vector2d &position) :
	Creature(position),
	chain_progress(-1),
	charges_current(Player_consts::BASE_CHARGES),
	charges_max(Player_consts::BASE_CHARGES),
	charges_time_elapsed(0),
	death_transition_performed(false)
{
	// Init sprite
	this->_init_sprite("player", {
		DEFAULT_ANIMATION_NAME,
		"move",
		"fire_chain_0",
		"fire_chain_1",
		"fire_chain_2",
		"fire_ult_0",
		"fire_ult_1"
		});

	// Init effect sprite
	this->_init_effect_sprite("player_effects", {
		DEFAULT_ANIMATION_NAME,
		"charged_jump"
		});

	// Init solid
	this->_init_solid(
		Player_consts::HITBOX_SIZE,
		SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
		Player_consts::MASS,
		Player_consts::FRICTION
	);

	// Init health
	this->_init_health(
		Faction::PLAYER,
		Player_consts::BASE_HP,
		Player_consts::BASE_REGEN,
		Player_consts::BASE_PHYS_RES,
		Player_consts::BASE_MAGIC_RES,
		Player_consts::BASE_CHAOS_RES
	);

	// Init members
	this->camera_trap_pos = this->position;
	this->camera_pos = this->camera_trap_pos;
}

TypeId Player::type_id() const { return TypeId::PLAYER; }

bool Player::update(Milliseconds elapsedTime) {
	if(!Creature::update(elapsedTime)) return false;

	this->effect_sprite->update(elapsedTime);

	this->_recalculate_stats();

	this->update_cameraTrapPos(elapsedTime);

	this->update_charges(elapsedTime);
	
	// Update correct state case
	const auto currentState = static_cast<State>(this->state_get());
	switch (currentState) {
	case State::STAND:	
		this->update_case_STAND(elapsedTime);
		break;

	case State::MOVE:
		this->update_case_MOVE(elapsedTime);
		break;

	case State::ATTACK:
		this->update_case_ATTACK(elapsedTime);
		break;

	case State::SKILL:
		this->update_case_ULT(elapsedTime);
		break;
	}

	// Handle other inputs
	auto &input = Game::ACCESS->input;

	// jumping down the platforms
	if (input.key_held(Controls::READ->DOWN) && input.key_pressed(Controls::READ->JUMP)) {
		this->solid->is_dropping_down = true;
		this->solid->is_grounded = false;

		this->dropping_down_sticky_delay.start(Player_consts::DROPPING_DOWN_STICKY_DELAY_DURATION);
	}
	else if (!input.key_held(Controls::READ->DOWN) && this->dropping_down_sticky_delay.finished()) {
		this->solid->is_dropping_down = false;
	}

	// Zoom-out
	/// Uncomment for debugging purposes
	/*if (input.key_held(sf::Keyboard::Key::R)) {
		Graphics::ACCESS->camera->set_zoom(2 * natural::ZOOM);
	}
	else {
		Graphics::ACCESS->camera->set_zoom(natural::ZOOM);
	}

	if (input.key_held(sf::Keyboard::Key::V)) {
		Game::ACCESS->timescale = 0.1;
	}
	else {
		Game::ACCESS->timescale = 1.0;
	}*/

	return true;
}

void Player::update_charges(Milliseconds elapsedTime) {
	// Watchung eye adds + 1 max charge (up to MAX_CHARGES_CAP = 5)
	const auto newMaxCharges = std::min(
		Player_consts::BASE_CHARGES + this->inventory.count("watching_eye"),
		Player_consts::MAX_CHARGES_CAP
	);

	// This ensures that charges get refilled when new Eye is picked up
	if (newMaxCharges != this->charges_max) {
		this->charges_max = newMaxCharges;
		this->charges_current = newMaxCharges;
	}

	if (this->charges_current < this->charges_max) {
		this->charges_time_elapsed += elapsedTime;

		if (this->charges_time_elapsed > Player_consts::CHARGE_CD) {
			this->charges_time_elapsed -= Player_consts::CHARGE_CD;

			++this->charges_current;
		}
	}
	else {
		this->charges_time_elapsed = 0.;
	}
}

void Player::update_case_STAND(Milliseconds elapsedTime) {
	using namespace Player_consts;

	Input& input = Game::ACCESS->input;

	// Every frame

	// jumping
	if (input.key_pressed(Controls::READ->JUMP)) {
		if (input.key_held(Controls::READ->DOWN)) this->jump_down_start();
		else if (input.key_released(Controls::READ->DOWN)) this->jump_down_end();
		else if (this->solid->is_grounded) this->jump();
	}

	// Transitions
	bool leftHeld = input.key_held(Controls::READ->LEFT);
	bool rightHeld = input.key_held(Controls::READ->RIGHT);

	if (leftHeld != rightHeld) {
		this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;
		this->_sprite->animation_play("move", true);
		this->state_change(State::MOVE);
	}

	if (input.key_held(Controls::READ->SKILL) && this->charges_current >= SKILL_CHARGE_COST) {
		this->charges_current -= SKILL_CHARGE_COST;

		this->chain_progress = 0;
		this->state_change(State::SKILL);
	}
	else if (input.mouse_held(Controls::READ->CHAIN)) {
		this->chain_progress = 0;
		this->state_change(State::ATTACK);
	}
}

void Player::update_case_MOVE(Milliseconds elapsedTime) {
	using namespace Player_consts;

	auto &input = Game::ACCESS->input;

	// Every frame
	if (input.key_pressed(Controls::READ->JUMP)) {
		if (input.key_held(Controls::READ->DOWN)) this->jump_down_start();
		else if (input.key_released(Controls::READ->DOWN)) this->jump_down_end();
		else if (this->solid->is_grounded) this->jump();
	}

	this->solid->applyForceTillMaxSpeed_Horizontal(
		Player_consts::MOVEMENT_FORCE,
		Player_consts::MOVEMENT_SPEED,
		this->orientation
	);

	// Transitions
	bool leftHeld = input.key_held(Controls::READ->LEFT);
	bool rightHeld = input.key_held(Controls::READ->RIGHT);

	if (leftHeld == rightHeld) {
		this->_sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
		this->state_change(State::STAND);
	}
	else {
		this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;
	}
	

	if (input.key_held(Controls::READ->SKILL) && this->charges_current >= SKILL_CHARGE_COST) {
		this->charges_current -= SKILL_CHARGE_COST;

		this->chain_progress = 0;
		this->state_change(State::SKILL);
	}
	else if (input.mouse_held(Controls::READ->CHAIN)) {
		this->chain_progress = 0;
		this->state_change(State::ATTACK);
	}
}

void Player::update_case_ATTACK(Milliseconds elapsedTime) {
	using namespace Player_consts;

	auto &sprite = this->_sprite;
	auto &input = Game::ACCESS->input;

	// Every frame
	const bool leftHeld = input.key_held(Controls::READ->LEFT);
	const bool rightHeld = input.key_held(Controls::READ->RIGHT);
	const auto movementInputPresent = (leftHeld != rightHeld);
	const auto orientationOfInput = leftHeld ? Orientation::LEFT : Orientation::RIGHT; // use only if input present

	if (movementInputPresent && orientationOfInput == this->orientation)
		this->solid->applyForceTillMaxSpeed_Horizontal(
			Player_consts::MOVEMENT_FORCE / 5.,
			Player_consts::MOVEMENT_SPEED / 5.,
			this->orientation
		);

	// Steps of attack chain
	switch (this->chain_progress) {
	case 0:
		if (movementInputPresent && orientationOfInput != this->orientation)
			this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;

		sprite->animation_play("fire_chain_0");
		++this->chain_progress;
		break;

	case 1:
		if (movementInputPresent && orientationOfInput != this->orientation)
			this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;

		if (sprite->animation_finished()) {
			// Deal AOE damage and knockback
			const auto attackHitbox = dRect(
				this->position.x - (this->orientation == Orientation::RIGHT ? fire::CHAIN_RANGE_BACK : fire::CHAIN_RANGE_FRONT),
				this->position.y - fire::CHAIN_RANGE_UP,
				fire::CHAIN_RANGE_FRONT + fire::CHAIN_RANGE_BACK,
				fire::CHAIN_RANGE_UP + fire::CHAIN_RANGE_DOWN
			); // nasty geometry

			for (auto& entity : Game::ACCESS->level->entities_killable)
				if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
					// Calculate dmg with respect to power shards
					// Power Shard - increases dmg by <x>% (additevely)
					const unsigned int numberOfPowerShards = this->inventory.count("twin_souls");
					const double dmgModifier = 1. + numberOfPowerShards * artifacts::POWER_SHARD_DMG_BOOST;
					const Damage damage = fire::CHAIN_0_DAMAGE * dmgModifier;

					entity->health->applyDamage(damage);

					if (this->health->faction != entity->health->faction) {
						const auto sign = helpers::sign(entity->position.x - this->position.x);
						entity->solid->addImpulse_Horizontal(fire::CHAIN_0_KNOCKBACK_X * sign);
						entity->solid->addImpulse_Up(fire::CHAIN_0_KNOCKBACK_Y);
					}
				}

			sprite->animation_play("fire_chain_1");
			++this->chain_progress;
		}
		break;

	case 2:
		if (sprite->animation_finished()) {
			// Deal AOE damage and knockback
			const auto attackHitbox = dRect(
				this->position.x - (this->orientation == Orientation::RIGHT ? fire::CHAIN_RANGE_BACK : fire::CHAIN_RANGE_FRONT),
				this->position.y - fire::CHAIN_RANGE_UP,
				fire::CHAIN_RANGE_FRONT + fire::CHAIN_RANGE_BACK,
				fire::CHAIN_RANGE_UP + fire::CHAIN_RANGE_DOWN
			); // nasty geometry

			for (auto& entity : Game::ACCESS->level->entities_killable)
				if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
					entity->health->applyDamage(fire::CHAIN_2_DAMAGE);

					if (this->health->faction != entity->health->faction) {
						const auto sign = helpers::sign(entity->position.x - this->position.x);
						entity->solid->addImpulse_Horizontal(fire::CHAIN_2_KNOCKBACK_X * sign);
						entity->solid->addImpulse_Up(fire::CHAIN_2_KNOCKBACK_Y);
					}
				}

			sprite->animation_play("fire_chain_2");
			this->chain_progress = -1;
		}
		break;

	default:
		break;
	}

	// Transitions
	if (this->chain_progress < 0) {
		if (movementInputPresent && sprite->animation_rushToEnd(1.5)) { // hurry up for movement
			this->orientation = orientationOfInput;

			sprite->animation_play("move", true);
			this->state_change(State::MOVE);
		}
		else if (sprite->animation_finished()) { // do animation at regular speed
			sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::STAND);
		}
	}

	if (input.key_held(Controls::READ->SKILL) && this->charges_current >= SKILL_CHARGE_COST) { // ult can interrupt attack
		this->charges_current -= SKILL_CHARGE_COST;

		this->chain_progress = 0;
		this->state_change(State::SKILL);
	}
}

void Player::update_case_ULT(Milliseconds elapsedTime) {
	auto &sprite = this->_sprite;
	auto &input = Game::ACCESS->input;

	// Every frame
	const bool leftHeld = input.key_held(Controls::READ->LEFT);
	const bool rightHeld = input.key_held(Controls::READ->RIGHT);
	const auto movementInputPresent = (leftHeld != rightHeld);
	const auto orientationOfInput = leftHeld ? Orientation::LEFT : Orientation::RIGHT; // use only if input present

	// Steps of attack
	switch (this->chain_progress) {
	case 0:
		// Teleport forward and change orientation
		this->horizontal_blink(this->orientation, fire::ULT_TELEPORT_RANGE);

		this->orientation = invert(this->orientation);

		sprite->animation_play("fire_ult_0");
		++this->chain_progress;
		break;

	case 1:
		if (sprite->animation_finished()) {
			// Deal AOE damage and knockback
			const auto attackHitbox = dRect(
				this->position.x - (this->orientation == Orientation::RIGHT ? fire::ULT_RANGE_BACK : fire::ULT_RANGE_FRONT),
				this->position.y - fire::ULT_RANGE_UP,
				fire::ULT_RANGE_FRONT + fire::ULT_RANGE_BACK,
				fire::ULT_RANGE_UP + fire::ULT_RANGE_DOWN
			); // nasty geometry

			for (auto& entity : Game::ACCESS->level->entities_killable)
				if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
					// Calculate dmg with respect to power shards
					// Power Shard - increases dmg by <x>% (additevely)
					const unsigned int numberOfPowerShards = this->inventory.count("twin_souls");
					const double dmgModifier = 1. + numberOfPowerShards * artifacts::POWER_SHARD_DMG_BOOST;
					const Damage damage = fire::ULT_DAMAGE * dmgModifier;

					entity->health->applyDamage(damage);

					if (this->health->faction != entity->health->faction) {
						const auto sign = helpers::sign(entity->position.x - this->position.x);
						entity->solid->addImpulse_Horizontal(fire::ULT_KNOCKBACK_X * sign);
						entity->solid->addImpulse_Up(fire::ULT_KNOCKBACK_Y);
					}
				}

			sprite->animation_play("fire_ult_1");
			this->chain_progress = -1;
		}
		break;
	}

	// Transitions
	if (this->chain_progress < 0) {
		if (movementInputPresent && sprite->animation_rushToEnd(1.5)) { // hurry up for movement
			this->orientation = orientationOfInput;

			sprite->animation_play("move", true);
			this->state_change(State::MOVE);
		}
		else if (sprite->animation_finished()) { // do animation at regular speed
			sprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			this->state_change(State::STAND);
		}
	}
}

void Player::jump() {
	using namespace Player_consts;

	double modifier = 1.;

	const bool powerfullJump = Game::ACCESS->input.key_held(Controls::READ->SHIFT) && this->charges_current >= JUMP_CHARGE_COST;

	if (powerfullJump) {
		this->charges_current -= JUMP_CHARGE_COST;

		modifier += this->inventory.count("spider_signet") * artifacts::SPIDER_SIGNET_JUMP_BOOST;

		this->effect_sprite->animation_play("charged_jump");
	}

	const auto jumpImpulse = JUMP_IMPULSE * modifier;

	this->solid->addImpulse_Up(jumpImpulse);
	this->solid->is_grounded = false;
}

void Player::jump_down_start() {
	this->solid->is_dropping_down = true;
	this->solid->is_grounded = false;
}

void Player::jump_down_end() {
	this->solid->is_dropping_down = false;
}

void Player::horizontal_blink(Orientation direction, double range) {
	const auto playerSize = this->solid->getHitbox().getSize();

	// Ensure we don't teleport through terrain, adjust tp range if necesary
	const Vector2 centerIndex = helpers::divide32(this->position);

	if (direction == Orientation::RIGHT) {
		// If there is no obstacles player right side will appear here
		double playerRightGoesTo = this->position.x + playerSize.x / 2. + range;

		// We only need to check 3-tile tall strip in a blink direction for collisions
		const int leftBound = centerIndex.x;
		const int rightBound = std::min(helpers::divide32(playerRightGoesTo), Game::READ->level->getSizeX() - 1);
		const int upperBound = std::max(centerIndex.y - 1, 0);
		const int lowerBound = std::min(centerIndex.y + 1, Game::READ->level->getSizeY() - 1);

		// Area that would be drawn if we 'continuously dragged' player hitbox to a new postion
		const dRect areaToCheck(
			this->solid->getHitbox().getCornerTopLeft(),
			playerSize + Vector2d(range, 0.)
		);

		// Go through tiles and determine where player would end up if we tried to 'continuously drag' hitbox
		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles)
						if (!hitboxRect.is_platform && hitboxRect.rect.overlapsWithRect(areaToCheck) && hitboxRect.rect.getLeft() < playerRightGoesTo)
							playerRightGoesTo = hitboxRect.rect.getLeft();
			}

		// Profit
		this->position.x = playerRightGoesTo - playerSize.x / 2.;
	}
	else {
		// If there is no obstacles player left side will appear here
		double playerLeftGoesTo = this->position.x - playerSize.x / 2. - range;

		// We only need to check 3-tile tall strip in a blink direction for collisions
		const int leftBound = std::max(helpers::divide32(playerLeftGoesTo), 0);
		const int rightBound = centerIndex.x;
		const int upperBound = std::max(centerIndex.y - 1, 0);
		const int lowerBound = std::min(centerIndex.y + 1, Game::READ->level->getSizeY() - 1);

		// Area that would be drawn if we 'continuously dragged' player hitbox to a new postion
		const dRect areaToCheck(
			this->solid->getHitbox().getCornerTopLeft() - Vector2d(range, 0.),
			playerSize + Vector2d(range, 0.)
		);

		// Go through tiles and determine where player would end up if we tried to 'continuously drag' hitbox
		for (int X = leftBound; X <= rightBound; ++X)
			for (int Y = upperBound; Y <= lowerBound; ++Y) {
				const auto tile = Game::ACCESS->level->getTile(X, Y);

				if (tile && tile->hitbox)
					for (const auto hitboxRect : tile->hitbox->rectangles)
						if (!hitboxRect.is_platform && hitboxRect.rect.overlapsWithRect(areaToCheck) && hitboxRect.rect.getRight() > playerLeftGoesTo)
							playerLeftGoesTo = hitboxRect.rect.getRight();
			}

		// Profit
		this->position.x = playerLeftGoesTo + playerSize.x / 2.;
	}
}

void Player::update_cameraTrapPos(Milliseconds elapsedTime) {
	if (this->position.x > this->camera_trap_pos.x + camera::TRAP_SIZE_RIGHT) {
		this->camera_trap_pos.x = this->position.x - camera::TRAP_SIZE_RIGHT;
	}
	else if (this->position.x < this->camera_trap_pos.x - camera::TRAP_SIZE_LEFT) {
		this->camera_trap_pos.x = this->position.x + camera::TRAP_SIZE_LEFT;
	}

	if (this->position.y > this->camera_trap_pos.y + camera::TRAP_SIZE_UP) {
		this->camera_trap_pos.y = this->position.y - camera::TRAP_SIZE_UP;
	}
	else if (this->position.y < this->camera_trap_pos.y - camera::TRAP_SIZE_DOWN) {
		this->camera_trap_pos.y = this->position.y + camera::TRAP_SIZE_DOWN;
	}

	const auto delta = this->camera_trap_pos - this->camera_pos;
	const auto deltaAbs = Vector2d(std::abs(delta.x), std::abs(delta.y));

	const double movementX = std::min(
		deltaAbs.x,
		std::max(camera::EPSILON_X, deltaAbs.x) * camera::SPEED_COEF_X * ms_to_sec(elapsedTime)
	); // min() prevent camera from oscillating around stable position due to rounding

	const double movementY = std::min(
		deltaAbs.y,
		std::max(camera::EPSILON_Y, deltaAbs.y) * camera::SPEED_COEF_Y * ms_to_sec(elapsedTime)
	); // min() prevent camera from oscillating around stable position due to rounding

	this->camera_pos.x += sign(delta.x) * movementX;
	this->camera_pos.y += sign(delta.y) * movementY;
}

void Player::draw() const {
	if (!this->creature_is_alive) return;

	Creature::draw();

	this->effect_sprite->draw();
}

void Player::deathTransition() {
	Creature::deathTransition();

	using namespace Player_consts;

	if (this->death_transition_performed) return;

	for (int i = 0; i < PARTICLE_COUNT; ++i) {
		Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
			this->position,
			Vector2d(rand_double(-PARTICLE_MAX_SPEED_X, PARTICLE_MAX_SPEED_X), rand_double(-PARTICLE_MAX_SPEED_Y, 0.)),
			colors::SH_BLACK,
			rand_double(PARTICLE_DURATION_MIN, PARTICLE_DURATION_MAX)
			));
	}

	Game::ACCESS->request_levelReload();
}

const Vector2d& Player::cameraTrap_getPosition() const {
	return this->camera_pos;
}

void Player::cameraTrap_center() {
	this->camera_trap_pos = this->position;
	this->camera_pos = this->camera_trap_pos;
}

std::string Player::get_state_name() {
	const auto currentState = static_cast<State>(this->state_get());

	switch (currentState) {
	case State::STAND:
		return "stand";

	case State::MOVE:
		return "move";

	case State::ATTACK:
		return "attack";

	case State::SKILL:
		return "skill";

	default:
		return "undefined";
	}
}

void Player::_init_effect_sprite(const std::string &folder, std::initializer_list<std::string> animationNames) {
	auto controllableSprite = std::make_unique<ControllableSprite>(
		this->position,
		true,
		false
		);

	bool defaultAnimationNotSet = true;

	for (auto &name : animationNames) {
		controllableSprite->animation_add(name, _parse_animation(PATH_TEXTURES_ENTITIES + folder + "/" + name));

		if (defaultAnimationNotSet && name == DEFAULT_ANIMATION_NAME) {
			controllableSprite->animation_play(DEFAULT_ANIMATION_NAME, true);
			defaultAnimationNotSet = false;
		}
	}

	this->effect_sprite = std::move(controllableSprite);
}

void Player::_recalculate_stats() {
	// Out of combat regen
	const double natural_regen_multi =
		(this->health->time_since_last_damage_received > Player_consts::OUT_OF_COMBAT_REGEN_TIMER)
		? Player_consts::OUT_OF_COMBAT_REGEN_MULTI
		: 0.;

	// Eldritch Battery - increases regen by <x>%
	const unsigned int numberOfEldritchBatteries = this->inventory.count("eldritch_battery");

	const double regenMulti = natural_regen_multi + numberOfEldritchBatteries * artifacts::ELDRITCH_BATTERY_REGEN_BOOST;
	this->health->setMulti(0, regenMulti, 0, 0, 0);

	// Bone Mask - every item reduces incoming physical dmg by <x>%
	const unsigned int numberOfBoneMasks = this->inventory.count("bone_mask");

	const sint physResFlat = static_cast<sint>(
		100. * (1. - std::pow(1. - artifacts::BONE_MASK_PHYS_DMG_REDUCTION, numberOfBoneMasks))
		);

	// Magic Negator - every item reduces incoming magic dmg by <x>%
	const unsigned int numberOfMagicNegators = this->inventory.count("magic_negator");

	const sint magicResFlat = static_cast<sint>(
		100. * (1. - std::pow(1. - artifacts::MAGIC_NEGATOR_MAGIC_DMG_REDUCTION, numberOfMagicNegators))
		);

	// Twin Souls - every item reduces incoming chaos dmg by <x>%
	const unsigned int numberOfTwinSouls = this->inventory.count("twin_souls");

	const sint chaosResFlat = static_cast<sint>(
		100. * (1. - std::pow(1. - artifacts::TWIN_SOULS_CHAOS_DMG_REDUCTION, numberOfTwinSouls))
		);

	this->health->setFlat(0, 0, physResFlat, magicResFlat, chaosResFlat);
}