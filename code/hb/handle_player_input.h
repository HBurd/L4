#ifndef HBHANDLE_PLAYER_INPUT_H
#define HBHANDLE_PLAYER_INPUT_H

#include "hb/player_control.h"
#include "hb/entities.h"
#include "hb/client.h"

void handle_player_input(
    PlayerControlState input,
    float delta_time,
    Physics *player_physics,
    PlayerInputBuffer *player_input_buffer,
    ClientData *client);

#ifdef FAST_BUILD
#include "handle_player_input.cpp"
#endif

#endif // include guard
