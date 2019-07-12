#pragma once

#include "hec.h"
#include "common/TransformComponent.h"
#include "common/PhysicsComponent.h"

#include "client/keyboard.h"

struct TrackingState
{
    EntityHandle guidance_target;
    bool track = false;
    bool stabilize = false;
};

Vec3 compute_target_tracking_torque(
    Transform player_transform,
    WorldSector player_reference,
    Physics player_physics,
    Transform target_transform,
    WorldSector target_reference);

Vec3 compute_stabilization_torque(
    Transform transform,
    Physics physics);

float compute_player_input_thrust(Keyboard kb);

Vec3 compute_player_input_torque(Keyboard kb);
