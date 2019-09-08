#pragma once

#include "common/TransformComponent.h"

#include "ccd/ccd.h"

struct BoundingBox
{
    float x1; // min
    float x2; // max
    float y1;
    float y2;
    float z1;
    float z2;
};

bool ray_intersect(const BoundingBox &aabb, Vec3 point, Vec3 direction, float *distance);

struct BoundingBoxData
{
    BoundingBox *bbox;
    Transform *transform;
    WorldSector *position_rf;
};

// Signature of ccd support function
void box_support(const void *box, const ccd_vec3_t *dir, ccd_vec3_t *vec);
