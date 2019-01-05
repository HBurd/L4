#ifndef HBPLAYER_CONTROL_H
#define HBPLAYER_CONTROL_H

#include "hbentities.h"
#include "hbkeyboard.h"

struct PlayerControlState
{
    float thrust = 0.0f;
    Rotor torque;
    bool shoot = false;
};

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Physics player,
    bool track,
    Physics target);

void player_control_update(Physics *physics, PlayerControlState control_state, float dt);

#endif // include guard
