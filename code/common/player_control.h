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

struct PlayerInputs
{
    bool leave_command_chair = false;
    ShipControls ship;
};


void get_ship_thrust(ShipControls input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque);

void apply_ship_inputs(ShipControls inputs, Transform *transform, Physics physics, float dt);

void handle_player_input(EntityManager *entity_manager, EntityHandle player_handle, PlayerInputs player_inputs, float dt);
