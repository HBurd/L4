#pragma once

#include "hec.h"
#include "client/ship_controller.h"
#include "client/keyboard.h"
#include "common/math.h"

struct LocalGameData
{
    float dt = 0.0f;
    Input input;

    EntityHandle player_handle;

    EntityManager *entity_manager;

    TrackingState tracking;

    LocalGameData(EntityManager *entity_manager_);
};
