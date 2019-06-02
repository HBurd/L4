#pragma once

#include "hec.h"
#include "player_control.h"

struct LocalGameData
{
    EntityHandle player_handle;
    EntityHandle player_ship_handle;

    EntityManager *entity_manager;

    TrackingState tracking;

    LocalGameData(EntityManager *entity_manager_);
};
