#pragma once

#include "client/local_game_data.h"
#include "client/keyboard.h"
#include "client/player_input.h"

#include "common/player_control.h"

const size_t MAX_PAST_INPUTS = 128;

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

ShipControls process_player_inputs(EntityManager *entity_manager, Keyboard kb, TrackingState tracking, EntityHandle ship_handle);
