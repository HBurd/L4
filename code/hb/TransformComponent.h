#ifndef HBTRANSFORM_H
#define HBTRANSFORM_H

#include "hb/math.h"

#define WORLD_SECTOR_COMPONENT WorldSector, world_sector, WORLD_SECTOR
struct WorldSector
{
    int64_t x = 0;
    int64_t y = 0;
    int64_t z = 0;
};

#define TRANSFORM_COMPONENT Transform, transform, TRANSFORM
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
    vector<WorldSector> *world_sector_components,
    vector<Transform> *transform_components,
    float dt);

Vec3 to_world_position(WorldSector sector, Transform transform);

#ifdef FAST_BUILD
#include "TransformComponent.cpp"
#endif

#endif // include guard
