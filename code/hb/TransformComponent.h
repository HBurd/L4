#ifndef HBTRANSFORM_H
#define HBTRANSFORM_H

#include "hb/math.h"

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

#ifdef FAST_BUILD
#include "TransformComponent.cpp"
#endif

#endif // include guard
