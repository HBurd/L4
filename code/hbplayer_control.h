#ifndef HBPLAYER_CONTROL_H
#define HBPLAYER_CONTROL_H

#include "hbentities.h"
#include "hbkeyboard.h"
#include "hbclient.h"

const size_t MAX_PAST_INPUTS = 128;

struct PlayerControlState
{
    float thrust = 0.0f;
    Rotor torque;
    bool shoot = false;
};

struct PastInput
{
    PlayerControlState input;   // input at frame start
    float dt = 0.0f;            // frame duration
};

struct PlayerInputBuffer
{
    PastInput inputs[MAX_PAST_INPUTS] = {};
    size_t next_input_idx = 0;

    void save_input(PlayerControlState control_state, float dt);
};

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Physics player,
    bool track,
    Physics target);

void player_control_update(Physics *physics, PlayerControlState control_state, float dt);
void handle_player_input(
    PlayerControlState input,
    float delta_time,
    Physics *player_physics,
    PlayerInputBuffer *player_input_buffer,
    ClientData *client);

#endif // include guard
