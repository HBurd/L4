#pragma once

#include "hec.h"
#include "common/TransformComponent.h"
#include "common/PhysicsComponent.h"

struct ShipControls
{
    float thrust = 0.0f;
    Vec3 torque;
    bool shoot = false;

    // clamps torque and thrust to physical limits
    void clamp();

    static constexpr float MAX_THRUST = 1.0f;
    static constexpr float MAX_TORQUE = 1.0f;
};

struct PlayerControls
{
    Vec3 movement;
    Rotor orientation;
};

struct PlayerInputs
{
    bool leave_command_chair = false;
    enum class InputType
    {
        PLAYER,
        SHIP,
    } type;

    // Only one of these should be used at a time
    PlayerControls player;
    ShipControls ship;
};


void get_ship_thrust(ShipControls input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque);

void apply_ship_inputs(ShipControls inputs, Transform *transform, Physics physics, float dt);

void handle_player_input(EntityManager *entity_manager, EntityHandle player_handle, PlayerInputs player_inputs, float dt);

void apply_input(PlayerInputs input, EntityHandle handle, float dt, EntityManager *entity_manager);

// Get the ship the player is controlling
// For now this is determined by the transform it is following
// TODO: Is there a better place for this?
EntityRef lookup_player_ship(EntityHandle player_handle, EntityManager *entity_manager);
