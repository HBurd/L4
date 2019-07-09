#include "common/collision.h"
#include "common/util.h"
#include "common/TransformComponent.h"
#include <float.h>

BoundingBox compute_bounding_box(const Transform &transform, const Mesh &mesh)
{
    BoundingBox result;
    result.x1 = result.y1 = result.z1 = FLT_MAX;
    result.x2 = result.y2 = result.z2 = -FLT_MAX;

    for (Vertex v : mesh.vertices)
    {
        Vec3 pos = mesh_to_sector(transform, v.position);

        result.x1 = HB_MIN(pos.x, result.x1);
        result.y1 = HB_MIN(pos.y, result.y1);
        result.z1 = HB_MIN(pos.z, result.z1);

        result.x2 = HB_MAX(pos.x, result.x2);
        result.y2 = HB_MAX(pos.y, result.y2);
        result.z2 = HB_MAX(pos.z, result.z2);
    }
    return result;
}

bool ray_intersect(const BoundingBox &aabb, Vec3 point, Vec3 direction, float *distance)
{
    bool intersect = false;
    float min_distance = FLT_MAX;
    if (direction.x != 0.0f)
    {
        // check intersection with x1 plane
        float dx1 = (aabb.x1 - point.x) / direction.x;
        // check if it's a valid intersection
        if (dx1 >= 0.0f
            && point.y + dx1 * direction.y >= aabb.y1
            && point.y + dx1 * direction.y <= aabb.y2
            && point.z + dx1 * direction.z >= aabb.z1
            && point.z + dx1 * direction.z <= aabb.z2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dx1);
        }

        float dx2 = (aabb.x2 - point.x) / direction.x;
        if (dx2 >= 0.0f
            && point.y + dx2 * direction.y >= aabb.y1
            && point.y + dx2 * direction.y <= aabb.y2
            && point.z + dx2 * direction.z >= aabb.z1
            && point.z + dx2 * direction.z <= aabb.z2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dx2);
        }
    }
    if (direction.y != 0.0f)
    {
        float dy1 = (aabb.y1 - point.y) / direction.y;
        if (dy1 >= 0.0f
            && point.x + dy1 * direction.x >= aabb.x1
            && point.x + dy1 * direction.x <= aabb.x2
            && point.z + dy1 * direction.z >= aabb.z1
            && point.z + dy1 * direction.z <= aabb.z2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dy1);
        }

        float dy2 = (aabb.y2 - point.y) / direction.y;
        if (dy2 >= 0.0f
            && point.x + dy2 * direction.x >= aabb.x1
            && point.x + dy2 * direction.x <= aabb.x2
            && point.z + dy2 * direction.z >= aabb.z1
            && point.z + dy2 * direction.z <= aabb.z2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dy2);
        }
    }
    if (direction.z != 0.0f)
    {
        float dz1 = (aabb.z1 - point.z) / direction.z;
        if (dz1 >= 0.0f
            && point.x + dz1 * direction.x >= aabb.x1
            && point.x + dz1 * direction.x <= aabb.x2
            && point.y + dz1 * direction.y >= aabb.y1
            && point.y + dz1 * direction.y <= aabb.y2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dz1);
        }

        float dz2 = (aabb.z2 - point.z) / direction.z;
        if (dz2 >= 0.0f
            && point.x + dz2 * direction.x >= aabb.x1
            && point.x + dz2 * direction.x <= aabb.x2
            && point.y + dz2 * direction.y >= aabb.y1
            && point.y + dz2 * direction.y <= aabb.y2)
        {
            intersect = true;
            min_distance = HB_MIN(min_distance, dz2);
        }
    }

    if (intersect && distance)
    {
        *distance = min_distance;
    }

    return intersect;
}
