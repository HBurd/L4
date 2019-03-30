#ifndef HBPLAYER_CONTROL_COMPONENT_H
#define HBPLAYER_CONTROL_COMPONENT_H

// TODO: this doesn't belong here
typedef size_t ClientId;

#define PLAYER_CONTROL_COMPONENT PlayerControl, player_control, PLAYER_CONTROL
struct PlayerControl
{
    ClientId client_id;
};

//#ifdef FAST_BUILD
//#include "PlayerControlComponent.cpp"
//#endif

#endif
