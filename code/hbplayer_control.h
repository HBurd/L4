#ifndef HBPLAYER_CONTROL_H
#define HBPLAYER_CONTROL_H

#include "hbentities.h"
#include "hbkeyboard.h"

struct PlayerControlState
{
    float thrust = 0.0f;
    Rotor torque;
};

// currently justs builds PlayerControlState based on kb
PlayerControlState player_control_get_state(Keyboard kb);

void player_control_update(Physics* physics, PlayerControlState control_state);

#endif // include guard
