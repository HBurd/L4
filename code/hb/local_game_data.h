#pragma once

#include "hec.h"
#include "hb/player_control.h"
#include "hb/math.h"

struct LocalGameData
{
    Rotor player_view_orientation;
    EntityHandle player_handle;
    EntityHandle player_ship_handle;

    EntityManager *entity_manager;

    TrackingState tracking;

    LocalGameData(EntityManager *entity_manager_);
};
