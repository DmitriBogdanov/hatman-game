#include "entity/player.h"

#include "entity/unique_s.h"        // for spawning projectile entities
#include "graphics/graphics.h"      // access to texture loading
#include "systems/controls.h"       // access to control keys
#include "systems/game.h"           // access to inputs
#include "utility/cx_math.hpp"      // for calculating jump speed
#include "utility/filepaths.hpp"    // path to textures
#include "utility/globalconsts.hpp" // physical consts


using namespace ntt;
using namespace ntt::player;

// # Player #
namespace player_consts {

constexpr auto hitbox_size = Vector2d(10., 30.);

constexpr double mass     = 100.;
constexpr double friction = 0.6;

constexpr double movement_speed        = 90.;
constexpr double movement_acceleration = 600.;
constexpr double movement_force        = mass * movement_acceleration;

constexpr double jump_speed   = cx_math::speed_corresponding_to_jump_height(physics::gravity_acceleration, 36.);
constexpr double jump_impulse = mass * jump_speed;

constexpr Milliseconds dropping_down_sticky_delay_duration =
    sec_to_ms(cx_math::sqrt(2 * physics::platform_epsilon / physics::gravity_acceleration) + 1e-4);
// required time is chosen based on guaranteeing that player falls down further than platform epsilon
// g t^2 / 2 = platform_epsilon   =>   t = sqrt(2 * platform_epsilon / g)

constexpr uint base_hp        = 1000;
constexpr sint base_regen     = 25;
constexpr sint base_phys_res  = 0;
constexpr sint base_magic_res = 0;
constexpr sint base_chaos_res = 0;

constexpr Milliseconds out_of_combat_regen_timer = sec_to_ms(10.);
constexpr double       out_of_combat_regen_multi = 8.;

// constexpr double STAND_TO_MOVE_ANIMATION_SPEEDUP = 3.;
// constexpr double STAND_TO_ATTACK_ANIMATION_SPEEDUP = 3.;

// Charges
constexpr uint base_charges    = 3;
constexpr uint max_charges_cap = 5;

constexpr uint skill_charge_cost = 2;
constexpr uint jump_charge_cost  = 1;

constexpr Milliseconds charge_cd = sec_to_ms(2);

// Death
constexpr int          particle_count        = 16;
constexpr double       particle_max_speed_x  = 200.;
constexpr double       particle_max_speed_y  = 250.;
constexpr Milliseconds particle_duration_min = sec_to_ms(1.);
constexpr Milliseconds particle_duration_max = sec_to_ms(6.);

} // namespace player_consts

namespace camera {

constexpr double trap_size_left  = 15.;
constexpr double trap_size_right = trap_size_left;
constexpr double trap_size_up    = 5.;
constexpr double trap_size_down  = 40.;
// camera moves in a way the player stays inside 'camera trap' rectangle
// gives camera smoother feel

constexpr double speed_coef_x = 20.;
constexpr double speed_coef_y = speed_coef_x;

constexpr double epsilon_x = 5.;
constexpr double epsilon_y = 5.;
// camera doesn't go slower if distance is less than epsilon
// prevents camera getting ever-asymptotically-closer, makes it go no less than certain speed

} // namespace camera

namespace fire {

// Chain
constexpr double chain_range_front = 30.;
constexpr double chain_range_back  = 2.;
constexpr double chain_range_up    = 20.;
constexpr double chain_range_down  = 12.;

const auto       chain_0_damage      = Damage(Faction::PLAYER, 0., 300., 0., 50.);
constexpr double chain_0_knockback_x = 20. * 100.;
constexpr double chain_0_knockback_y = 0. * 100.;

const auto       chain_2_damage      = Damage(Faction::PLAYER, 0., 400., 0., 50.);
constexpr double chain_2_knockback_x = 120. * 100.;
constexpr double chain_2_knockback_y = 110. * 100.;

// Ult
constexpr double ult_teleport_range = 50.;

constexpr double ult_range_front = 40.;
constexpr double ult_range_back  = 2.;
constexpr double ult_range_up    = 20.;
constexpr double ult_range_down  = 12.;

const auto ult_damage = Damage(Faction::PLAYER, 0., 750., 0., 50.);

constexpr double ult_knockback_x = 100. * 100.;
constexpr double ult_knockback_y = 200. * 100.;

} // namespace fire



Player::Player(const Vector2d& position)
    : Creature(position), charges_current(player_consts::base_charges), charges_max(player_consts::base_charges) {
    // Init sprite
    this->init_sprite("player", {default_animation_name, "move", "fire_chain_0", "fire_chain_1", "fire_chain_2",
                                  "fire_ult_0", "fire_ult_1"});

    // Init effect sprite
    this->init_effect_sprite("player_effects", {default_animation_name, "charged_jump"});

    // Init solid
    this->init_solid(player_consts::hitbox_size, SolidFlags::SOLID | SolidFlags::AFFECTED_BY_GRAVITY,
                      player_consts::mass, player_consts::friction);

    // Init health
    this->init_health(Faction::PLAYER, player_consts::base_hp, player_consts::base_regen, player_consts::base_phys_res,
                       player_consts::base_magic_res, player_consts::base_chaos_res);

    // Init members
    this->camera_trap_pos = this->position;
    this->camera_pos      = this->camera_trap_pos;

    this->state_change(State::STAND);
}

TypeId Player::type_id() const { return TypeId::PLAYER; }

bool Player::update(Milliseconds elapsedTime) {
    if (!Creature::update(elapsedTime)) return false;

    this->effect_sprite->update(elapsedTime);

    this->recalculate_stats();

    this->update_camera_trap_pos(elapsedTime);

    this->update_charges(elapsedTime);

    // Update correct state case
    const auto currentState = static_cast<State>(this->state_get());
    switch (currentState) {
    case State::STAND: this->update_case_stand(elapsedTime); break;

    case State::MOVE: this->update_case_move(elapsedTime); break;

    case State::ATTACK: this->update_case_attack(elapsedTime); break;

    case State::SKILL: this->update_case_ult(elapsedTime); break;
    }

    // Handle other inputs
    auto& input = Game::ACCESS->input;

    // jumping down the platforms
    if (input.key_held(Controls::READ->DOWN) && input.key_pressed(Controls::READ->JUMP)) {
        this->solid->is_dropping_down = true;
        this->solid->is_grounded      = false;

        this->dropping_down_sticky_delay.start(player_consts::dropping_down_sticky_delay_duration);
    } else if (!input.key_held(Controls::READ->DOWN) && this->dropping_down_sticky_delay.finished()) {
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
    // Watching eye adds + 1 max charge (up to MAX_CHARGES_CAP = 5)
    const auto newMaxCharges =
        std::min(player_consts::base_charges + this->inventory.count("watching_eye"), player_consts::max_charges_cap);

    // This ensures that charges get refilled when new Eye is picked up
    if (newMaxCharges != this->charges_max) {
        this->charges_max     = newMaxCharges;
        this->charges_current = newMaxCharges;
    }

    if (this->charges_current < this->charges_max) {
        this->charges_time_elapsed += elapsedTime;

        if (this->charges_time_elapsed > player_consts::charge_cd) {
            this->charges_time_elapsed -= player_consts::charge_cd;

            ++this->charges_current;
        }
    } else {
        this->charges_time_elapsed = 0.;
    }
}

void Player::update_case_stand([[maybe_unused]] Milliseconds elapsedTime) {
    using namespace player_consts;

    Input& input = Game::ACCESS->input;

    // Every frame

    // jumping
    if (input.key_pressed(Controls::READ->JUMP)) {
        if (input.key_held(Controls::READ->DOWN)) this->jump_down_start();
        else if (input.key_released(Controls::READ->DOWN)) this->jump_down_end();
        else if (this->solid->is_grounded) this->jump();
    }

    // Transitions
    bool leftHeld  = input.key_held(Controls::READ->LEFT);
    bool rightHeld = input.key_held(Controls::READ->RIGHT);

    if (leftHeld != rightHeld) {
        this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;
        this->_sprite->animation_play("move", true);
        this->state_change(State::MOVE);
    }

    if (input.key_held(Controls::READ->SKILL) && this->charges_current >= skill_charge_cost) {
        this->charges_current -= skill_charge_cost;

        this->chain_progress = 0;
        this->state_change(State::SKILL);
    } else if (input.mouse_held(Controls::READ->CHAIN)) {
        this->chain_progress = 0;
        this->state_change(State::ATTACK);
    }
}

void Player::update_case_move([[maybe_unused]] Milliseconds elapsedTime) {
    using namespace player_consts;

    auto& input = Game::ACCESS->input;

    // Every frame
    if (input.key_pressed(Controls::READ->JUMP)) {
        if (input.key_held(Controls::READ->DOWN)) this->jump_down_start();
        else if (input.key_released(Controls::READ->DOWN)) this->jump_down_end();
        else if (this->solid->is_grounded) this->jump();
    }

    this->solid->applyForceTillMaxSpeed_Horizontal(player_consts::movement_force, player_consts::movement_speed,
                                                   this->orientation);

    // Transitions
    bool leftHeld  = input.key_held(Controls::READ->LEFT);
    bool rightHeld = input.key_held(Controls::READ->RIGHT);

    if (leftHeld == rightHeld) {
        this->_sprite->animation_play(default_animation_name, true);
        this->state_change(State::STAND);
    } else {
        this->orientation = leftHeld ? Orientation::LEFT : Orientation::RIGHT;
    }


    if (input.key_held(Controls::READ->SKILL) && this->charges_current >= skill_charge_cost) {
        this->charges_current -= skill_charge_cost;

        this->chain_progress = 0;
        this->state_change(State::SKILL);
    } else if (input.mouse_held(Controls::READ->CHAIN)) {
        this->chain_progress = 0;
        this->state_change(State::ATTACK);
    }
}

void Player::update_case_attack([[maybe_unused]] Milliseconds elapsedTime) {
    using namespace player_consts;

    auto& sprite = this->_sprite;
    auto& input  = Game::ACCESS->input;

    // Every frame
    const bool leftHeld             = input.key_held(Controls::READ->LEFT);
    const bool rightHeld            = input.key_held(Controls::READ->RIGHT);
    const auto movementInputPresent = (leftHeld != rightHeld);
    const auto orientationOfInput   = leftHeld ? Orientation::LEFT : Orientation::RIGHT; // use only if input present

    if (movementInputPresent && orientationOfInput == this->orientation)
        this->solid->applyForceTillMaxSpeed_Horizontal(player_consts::movement_force / 5.,
                                                       player_consts::movement_speed / 5., this->orientation);

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
            const auto attackHitbox =
                dRect(this->position.x -
                          (this->orientation == Orientation::RIGHT ? fire::chain_range_back : fire::chain_range_front),
                      this->position.y - fire::chain_range_up, fire::chain_range_front + fire::chain_range_back,
                      fire::chain_range_up + fire::chain_range_down); // nasty geometry

            for (auto& entity : Game::ACCESS->level->entities_killable)
                if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
                    // Calculate dmg with respect to power shards
                    // Power Shard - increases dmg by <x>% (additively)
                    const unsigned int numberOfPowerShards = this->inventory.count("twin_souls");
                    const double       dmgModifier = 1. + numberOfPowerShards * artifacts::power_shard_dmg_boost;
                    const Damage       damage      = fire::chain_0_damage * dmgModifier;

                    entity->health->applyDamage(damage);

                    if (this->health->faction != entity->health->faction) {
                        const auto sign = helpers::sign(entity->position.x - this->position.x);
                        entity->solid->addImpulse_Horizontal(fire::chain_0_knockback_x * sign);
                        entity->solid->addImpulse_Up(fire::chain_0_knockback_y);
                    }
                }

            sprite->animation_play("fire_chain_1");
            ++this->chain_progress;
        }
        break;

    case 2:
        if (sprite->animation_finished()) {
            // Deal AOE damage and knockback
            const auto attackHitbox =
                dRect(this->position.x -
                          (this->orientation == Orientation::RIGHT ? fire::chain_range_back : fire::chain_range_front),
                      this->position.y - fire::chain_range_up, fire::chain_range_front + fire::chain_range_back,
                      fire::chain_range_up + fire::chain_range_down); // nasty geometry

            for (auto& entity : Game::ACCESS->level->entities_killable)
                if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
                    entity->health->applyDamage(fire::chain_2_damage);

                    if (this->health->faction != entity->health->faction) {
                        const auto sign = helpers::sign(entity->position.x - this->position.x);
                        entity->solid->addImpulse_Horizontal(fire::chain_2_knockback_x * sign);
                        entity->solid->addImpulse_Up(fire::chain_2_knockback_y);
                    }
                }

            sprite->animation_play("fire_chain_2");
            this->chain_progress = -1;
        }
        break;

    default: break;
    }

    // Transitions
    if (this->chain_progress < 0) {
        if (movementInputPresent && sprite->animation_rushToEnd(1.5)) { // hurry up for movement
            this->orientation = orientationOfInput;

            sprite->animation_play("move", true);
            this->state_change(State::MOVE);
        } else if (sprite->animation_finished()) { // do animation at regular speed
            sprite->animation_play(default_animation_name, true);
            this->state_change(State::STAND);
        }
    }

    if (input.key_held(Controls::READ->SKILL) &&
        this->charges_current >= skill_charge_cost) { // ult can interrupt attack
        this->charges_current -= skill_charge_cost;

        this->chain_progress = 0;
        this->state_change(State::SKILL);
    }
}

void Player::update_case_ult([[maybe_unused]] Milliseconds elapsedTime) {
    auto& sprite = this->_sprite;
    auto& input  = Game::ACCESS->input;

    // Every frame
    const bool leftHeld             = input.key_held(Controls::READ->LEFT);
    const bool rightHeld            = input.key_held(Controls::READ->RIGHT);
    const auto movementInputPresent = (leftHeld != rightHeld);
    const auto orientationOfInput   = leftHeld ? Orientation::LEFT : Orientation::RIGHT; // use only if input present

    // Steps of attack
    switch (this->chain_progress) {
    case 0:
        // Teleport forward and change orientation
        this->horizontal_blink(this->orientation, fire::ult_teleport_range);

        this->orientation = invert(this->orientation);

        sprite->animation_play("fire_ult_0");
        ++this->chain_progress;
        break;

    case 1:
        if (sprite->animation_finished()) {
            // Deal AOE damage and knockback
            const auto attackHitbox =
                dRect(this->position.x -
                          (this->orientation == Orientation::RIGHT ? fire::ult_range_back : fire::ult_range_front),
                      this->position.y - fire::ult_range_up, fire::ult_range_front + fire::ult_range_back,
                      fire::ult_range_up + fire::ult_range_down); // nasty geometry

            for (auto& entity : Game::ACCESS->level->entities_killable)
                if (attackHitbox.overlapsWithRect(entity->solid->getHitbox())) {
                    // Calculate dmg with respect to power shards
                    // Power Shard - increases dmg by <x>% (additively)
                    const unsigned int numberOfPowerShards = this->inventory.count("twin_souls");
                    const double       dmgModifier = 1. + numberOfPowerShards * artifacts::power_shard_dmg_boost;
                    const Damage       damage      = fire::ult_damage * dmgModifier;

                    entity->health->applyDamage(damage);

                    if (this->health->faction != entity->health->faction) {
                        const auto sign = helpers::sign(entity->position.x - this->position.x);
                        entity->solid->addImpulse_Horizontal(fire::ult_knockback_x * sign);
                        entity->solid->addImpulse_Up(fire::ult_knockback_y);
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
        } else if (sprite->animation_finished()) { // do animation at regular speed
            sprite->animation_play(default_animation_name, true);
            this->state_change(State::STAND);
        }
    }
}

void Player::jump() {
    using namespace player_consts;

    double modifier = 1.;

    const bool powerfullJump =
        Game::ACCESS->input.key_held(Controls::READ->SHIFT) && this->charges_current >= jump_charge_cost;

    if (powerfullJump) {
        this->charges_current -= jump_charge_cost;

        modifier += this->inventory.count("spider_signet") * artifacts::spider_signet_jump_boost;

        this->effect_sprite->animation_play("charged_jump");
    }

    const auto jumpImpulse = jump_impulse * modifier;

    this->solid->addImpulse_Up(jumpImpulse);
    this->solid->is_grounded = false;
}

void Player::jump_down_start() {
    this->solid->is_dropping_down = true;
    this->solid->is_grounded      = false;
}

void Player::jump_down_end() { this->solid->is_dropping_down = false; }

void Player::horizontal_blink(Orientation direction, double range) {
    const auto playerSize = this->solid->getHitbox().getSize();

    // Ensure we don't teleport through terrain, adjust tp range if necessary
    const Vector2 centerIndex = helpers::divide32(this->position);

    if (direction == Orientation::RIGHT) {
        // If there is no obstacles player right side will appear here
        double playerRightGoesTo = this->position.x + playerSize.x / 2. + range;

        // We only need to check 3-tile tall strip in a blink direction for collisions
        const int leftBound  = centerIndex.x;
        const int rightBound = std::min(helpers::divide32(playerRightGoesTo), Game::READ->level->getSizeX() - 1);
        const int upperBound = std::max(centerIndex.y - 1, 0);
        const int lowerBound = std::min(centerIndex.y + 1, Game::READ->level->getSizeY() - 1);

        // Area that would be drawn if we 'continuously dragged' player hitbox to a new postion
        const dRect areaToCheck(this->solid->getHitbox().getCornerTopLeft(), playerSize + Vector2d(range, 0.));

        // Go through tiles and determine where player would end up if we tried to 'continuously drag' hitbox
        for (int X = leftBound; X <= rightBound; ++X)
            for (int Y = upperBound; Y <= lowerBound; ++Y) {
                const auto tile = Game::ACCESS->level->getTile(X, Y);

                if (tile && tile->hitbox)
                    for (const auto hitboxRect : tile->hitbox->rectangles)
                        if (!hitboxRect.is_platform && hitboxRect.rect.overlapsWithRect(areaToCheck) &&
                            hitboxRect.rect.getLeft() < playerRightGoesTo)
                            playerRightGoesTo = hitboxRect.rect.getLeft();
            }

        // Profit
        this->position.x = playerRightGoesTo - playerSize.x / 2.;
    } else {
        // If there is no obstacles player left side will appear here
        double playerLeftGoesTo = this->position.x - playerSize.x / 2. - range;

        // We only need to check 3-tile tall strip in a blink direction for collisions
        const int leftBound  = std::max(helpers::divide32(playerLeftGoesTo), 0);
        const int rightBound = centerIndex.x;
        const int upperBound = std::max(centerIndex.y - 1, 0);
        const int lowerBound = std::min(centerIndex.y + 1, Game::READ->level->getSizeY() - 1);

        // Area that would be drawn if we 'continuously dragged' player hitbox to a new postion
        const dRect areaToCheck(this->solid->getHitbox().getCornerTopLeft() - Vector2d(range, 0.),
                                playerSize + Vector2d(range, 0.));

        // Go through tiles and determine where player would end up if we tried to 'continuously drag' hitbox
        for (int X = leftBound; X <= rightBound; ++X)
            for (int Y = upperBound; Y <= lowerBound; ++Y) {
                const auto tile = Game::ACCESS->level->getTile(X, Y);

                if (tile && tile->hitbox)
                    for (const auto hitboxRect : tile->hitbox->rectangles)
                        if (!hitboxRect.is_platform && hitboxRect.rect.overlapsWithRect(areaToCheck) &&
                            hitboxRect.rect.getRight() > playerLeftGoesTo)
                            playerLeftGoesTo = hitboxRect.rect.getRight();
            }

        // Profit
        this->position.x = playerLeftGoesTo + playerSize.x / 2.;
    }
}

void Player::update_camera_trap_pos(Milliseconds elapsedTime) {
    if (this->position.x > this->camera_trap_pos.x + camera::trap_size_right) {
        this->camera_trap_pos.x = this->position.x - camera::trap_size_right;
    } else if (this->position.x < this->camera_trap_pos.x - camera::trap_size_left) {
        this->camera_trap_pos.x = this->position.x + camera::trap_size_left;
    }

    if (this->position.y > this->camera_trap_pos.y + camera::trap_size_up) {
        this->camera_trap_pos.y = this->position.y - camera::trap_size_up;
    } else if (this->position.y < this->camera_trap_pos.y - camera::trap_size_down) {
        this->camera_trap_pos.y = this->position.y + camera::trap_size_down;
    }

    const auto delta    = this->camera_trap_pos - this->camera_pos;
    const auto deltaAbs = Vector2d(std::abs(delta.x), std::abs(delta.y));

    const double movementX = std::min(
        deltaAbs.x,
        std::max(camera::epsilon_x, deltaAbs.x) * camera::speed_coef_x *
            ms_to_sec(elapsedTime)); // min() prevent camera from oscillating around stable position due to rounding

    const double movementY = std::min(
        deltaAbs.y,
        std::max(camera::epsilon_y, deltaAbs.y) * camera::speed_coef_y *
            ms_to_sec(elapsedTime)); // min() prevent camera from oscillating around stable position due to rounding

    this->camera_pos.x += sign(delta.x) * movementX;
    this->camera_pos.y += sign(delta.y) * movementY;
}

void Player::draw() const {
    if (!this->creature_is_alive) return;

    Creature::draw();

    this->effect_sprite->draw();
}

void Player::death_transition() {
    Creature::death_transition();

    using namespace player_consts;

    if (this->death_transition_performed) return;

    for (int i = 0; i < particle_count; ++i) {
        Game::ACCESS->level->spawn(std::make_unique<s::particle::OnDeathParticle>(
            this->position,
            Vector2d(rand_double(-particle_max_speed_x, particle_max_speed_x), rand_double(-particle_max_speed_y, 0.)),
            colors::SH_BLACK, rand_double(particle_duration_min, particle_duration_max)));
    }

    Game::ACCESS->request_levelReload();
}

const Vector2d& Player::camera_trap_get_pos() const { return this->camera_pos; }

void Player::camera_trap_center() {
    this->camera_trap_pos = this->position;
    this->camera_pos      = this->camera_trap_pos;
}

std::string Player::get_state_name() {
    const auto currentState = static_cast<State>(this->state_get());

    switch (currentState) {
    case State::STAND: return "stand";

    case State::MOVE: return "move";

    case State::ATTACK: return "attack";

    case State::SKILL: return "skill";

    default: return "undefined";
    }
}

void Player::init_effect_sprite(const std::string& folder, std::initializer_list<std::string> animationNames) {
    auto controllableSprite = std::make_unique<ControllableSprite>(this->position, true, false);

    bool defaultAnimationNotSet = true;

    for (auto& name : animationNames) {
        controllableSprite->animation_add(name, parse_animation(paths::textures_entities + folder + "/" + name));

        if (defaultAnimationNotSet && name == default_animation_name) {
            controllableSprite->animation_play(default_animation_name, true);
            defaultAnimationNotSet = false;
        }
    }

    this->effect_sprite = std::move(controllableSprite);
}

void Player::recalculate_stats() {
    // Out of combat regen
    const double natural_regen_multi =
        (this->health->time_since_last_damage_received > player_consts::out_of_combat_regen_timer)
            ? player_consts::out_of_combat_regen_multi
            : 0.;

    // Eldritch Battery - increases regen by <x>%
    const unsigned int numberOfEldritchBatteries = this->inventory.count("eldritch_battery");

    const double regenMulti = natural_regen_multi + numberOfEldritchBatteries * artifacts::eldritch_battery_regen_boost;
    this->health->setMulti(0, regenMulti, 0, 0, 0);

    // Bone Mask - every item reduces incoming physical dmg by <x>%
    const unsigned int numberOfBoneMasks = this->inventory.count("bone_mask");

    const sint physResFlat =
        static_cast<sint>(100. * (1. - std::pow(1. - artifacts::bone_mask_phys_dmg_reduction, numberOfBoneMasks)));

    // Magic Negator - every item reduces incoming magic dmg by <x>%
    const unsigned int numberOfMagicNegators = this->inventory.count("magic_negator");

    const sint magicResFlat = static_cast<sint>(
        100. * (1. - std::pow(1. - artifacts::magic_negator_magic_dmg_reduction, numberOfMagicNegators)));

    // Twin Souls - every item reduces incoming chaos dmg by <x>%
    const unsigned int numberOfTwinSouls = this->inventory.count("twin_souls");

    const sint chaosResFlat =
        static_cast<sint>(100. * (1. - std::pow(1. - artifacts::twin_souls_chaos_dmg_reduction, numberOfTwinSouls)));

    this->health->setFlat(0, 0, physResFlat, magicResFlat, chaosResFlat);
}