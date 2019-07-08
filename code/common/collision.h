#pragma once

#include "common/mesh.h"

struct BoundingBox
{
    float x1; // min
    float x2; // max
    float y1;
    float y2;
    float z1;
    float z2;
};

BoundingBox compute_bounding_box(const Transform &transform, const Mesh &mesh);

bool ray_intersect(const BoundingBox &aabb, Vec3 point, Vec3 direction, float *distance);
