#pragma once

#include "hec.h"
#include "common/player_control.h"
#include "common/math.h"

struct LocalGameData
{
    float dt = 0.0f;

    Rotor player_view_orientation;
    EntityHandle player_handle;
    EntityHandle player_ship_handle;

    EntityManager *entity_manager;

    TrackingState tracking;

    LocalGameData(EntityManager *entity_manager_);
};
