#pragma once

#include "hb/packets.h"
#include "hb/client.h"
#include "hb/local_game_data.h"

void handle_entity_create(LocalGameData *game, ClientData *client, GamePacketIn packet);

void handle_physics_sync(LocalGameData *game, PlayerInputBuffer *past_inputs, GamePacketIn packet);
