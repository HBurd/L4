#pragma once

#include "hec.h"
#include "client/keyboard.h"
#include "common/TransformComponent.h"
#include "common/PhysicsComponent.h"

const size_t MAX_PAST_INPUTS = 128;

struct TrackingState
{
    EntityHandle guidance_target;
    bool track = false;
    bool stabilize = false;
};

struct ShipControls
{
    float thrust = 0.0f;
    Vec3 torque;
    bool shoot = false;

    // clamps torque and thrust to physical limits
    void clamp();
};

struct PlayerInputs
{
    bool leave_command_chair = false;
    ShipControls ship;
};

struct PastInput
{
    ShipControls input;   // input at frame start
    float dt = 0.0f;            // frame duration
    uint32_t sequence_number = 0;
};

struct PlayerInputBuffer
{
    PastInput inputs[MAX_PAST_INPUTS] = {};
    uint32_t next_seq_num = 0;
    uint32_t last_received_seq_num = 0;

    void save_input(ShipControls control_state, float dt);
};

Vec3 compute_target_tracking_torque(
    Transform player_transform,
    Physics player_physics,
    Transform target_transform);

Vec3 compute_stabilization_torque(
    Transform transform,
    Physics physics);

Vec3 compute_player_input_torque(Keyboard kb);

float compute_player_input_thrust(Keyboard kb);

void get_ship_thrust(ShipControls input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque);

void apply_ship_inputs(ShipControls inputs, Transform *transform, Physics physics);

ShipControls ship_control(EntityManager *entity_manager, Keyboard kb, TrackingState tracking, EntityHandle ship_handle);

void handle_player_input(EntityManager *entity_manager, EntityHandle player_handle, PlayerInputs player_inputs);
