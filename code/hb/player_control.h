#ifndef HBPLAYER_CONTROL_H
#define HBPLAYER_CONTROL_H

#include "hb/components.h"
#include "hec.h"
#include "hb/keyboard.h"

const size_t MAX_PAST_INPUTS = 128;

struct PlayerControlState
{
    float thrust = 0.0f;
    Vec3 torque;
    bool shoot = false;

    // clamps torque and thrust to physical limits
    void clamp();
};

struct PastInput
{
    PlayerControlState input;   // input at frame start
    float dt = 0.0f;            // frame duration
    uint32_t sequence_number = 0;
};

struct PlayerInputBuffer
{
    PastInput inputs[MAX_PAST_INPUTS] = {};
    uint32_t next_seq_num = 0;
    uint32_t last_received_seq_num = 0;

    void save_input(PlayerControlState control_state, float dt);
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

void get_ship_thrust(PlayerControlState input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque);

#endif // include guard
