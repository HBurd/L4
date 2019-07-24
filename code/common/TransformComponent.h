#pragma once

#include "common/math.h"
#include <stdlib.h>
#include <stdint.h>

struct WorldSector
{
    int64_t x = 0;
    int64_t y = 0;
    int64_t z = 0;
};

struct Transform
{
    Vec3 position;
    Vec3 velocity;
    Rotor orientation;
    Vec3 angular_velocity;
    Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);

    Transform() = default;
    Transform(Vec3 transform_position);
};

void update_transform_components(
    WorldSector *world_sectors,
    Transform *transforms,
    size_t num_components,
    float dt);

void update_transform(
    WorldSector *position_rf,
    Transform *transform,
    float dt);

Vec3 mesh_to_sector(Transform transform, Vec3 pos);

Vec3 relative_to_sector(const WorldSector &from, const WorldSector &sector, const Vec3 &position);
