#pragma once

#include "common/packets.h"
#include "client/client.h"
#include "client/local_game_data.h"

void handle_entity_create(LocalGameData *game, ClientData *client, GamePacketIn packet);

void handle_physics_sync(LocalGameData *game, PlayerInputBuffer *past_inputs, GamePacketIn packet);
