#ifndef HBPLAYER_CONTROL_H
#define HBPLAYER_CONTROL_H

#include "hb/entities.h"
#include "hb/keyboard.h"

const size_t MAX_PAST_INPUTS = 128;

struct PlayerControlState
{
    float thrust = 0.0f;
    Vec3 torque;
    bool shoot = false;
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

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Transform player,
    bool track,
    Transform target);

void get_ship_thrust(PlayerControlState input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque);

#ifdef FAST_BUILD
#include "player_control.cpp"
#endif

#endif // include guard
