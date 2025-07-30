// ____________________ HEADER SUMMARY ____________________

// Player entity and its behavior.

// ____________________ IMPLEMENTATION ____________________

#pragma once

#include "entity/type_m.h"     // 'Creature' base class
#include "modules/inventory.h" // 'Inventory' module



namespace artifacts {

// Regen
constexpr double eldritch_battery_regen_boost = 0.10;

// Damage
constexpr double power_shard_dmg_boost = 0.1;

// Utility
constexpr double spider_signet_jump_boost = 0.08;

// Resistances
constexpr double bone_mask_phys_dmg_reduction      = 0.2;
constexpr double magic_negator_magic_dmg_reduction = 0.2;
constexpr double twin_souls_chaos_dmg_reduction    = 0.2;

} // namespace artifacts



// ntt::player::
// - Contains player class
// - Unlike other creatures doesn't get erased upon death, but rather 'fakes' the effects of it
namespace ntt::player {
// # Player #
class Player : public m_type::Creature {
public:
    Player() = delete;

    Player(const Vector2d& position);

    virtual ~Player() = default;

    TypeId type_id() const override;

    bool update(Milliseconds elapsedTime) override;

    enum class State { STAND, MOVE, ATTACK, SKILL };

    void update_charges(Milliseconds elapsedTime);

    void update_case_stand(Milliseconds elapsedTime);
    void update_case_move(Milliseconds elapsedTime);
    void update_case_attack(Milliseconds elapsedTime);
    void update_case_ult(Milliseconds elapsedTime);

    void jump();
    void jump_down_start(); // jump down the platform
    void jump_down_end();
    void horizontal_blink(Orientation direction, double range); // teleport with respect to terrain

    void update_camera_trap_pos(Milliseconds elapsedTime);

    void draw() const override; // also draws effect_sprite

    void death_transition() override;

    const Vector2d& camera_trap_get_pos() const;
    void            camera_trap_center(); // centers trap at the player, used during level change

    std::string get_state_name(); // used in debug display

    Inventory inventory{};

    uint         charges_current{};
    uint         charges_max{};
    Milliseconds charges_time_elapsed = 0;

private:
    void init_effect_sprite(const std::string& folder, std::initializer_list<std::string> animationNames);

    void recalculate_stats(); // every update recalculate stats base on artifacts

    std::unique_ptr<HealthbarDisplay> healthbar_display{};

    Vector2d camera_trap_pos{};
    Vector2d camera_pos{};

    int chain_progress = -1; // index of current attack in chain

    std::unique_ptr<ControllableSprite> effect_sprite{};
    // secondary sprite used to render effects (like powerfull jump) without
    // interfering with main animations

    bool death_transition_performed = false;

    Timer dropping_down_sticky_delay{}; // delays setting 'solid->is_dropping_down' after player releases 'S' which
                                        //  prevents character from teleporting slightly up after releasing 'S' too fast
};

} // namespace ntt::player