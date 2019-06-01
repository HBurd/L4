#pragma once

#include "hec.h"
#include "hb/packets.h"
#include "hb/client.h"

struct LocalGameData
{
    EntityHandle player_handle;
    EntityHandle player_ship_handle;

    EntityManager *entity_manager;

    // Tracking data
    EntityHandle guidance_target;
    bool track = false;
    bool stabilize = false;

    LocalGameData(EntityManager *entity_manager_);
};

void handle_entity_create(LocalGameData *game, ClientData *client, GamePacketIn packet);

void handle_physics_sync(LocalGameData *game, PlayerInputBuffer *past_inputs, GamePacketIn packet);
