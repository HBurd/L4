#include "common/collision.h"
#include "common/util.h"
#include "common/TransformComponent.h"
#include <float.h>

void box_support(const void *box_p, const ccd_vec3_t *dir, ccd_vec3_t *vec)
{
    BoundingBoxData *box = (BoundingBoxData*)box_p;
    BoundingBox *bbox = box->bbox;
    Transform *transform = box->transform;
    WorldSector *position_rf = box->position_rf;

    // TODO: pick reference frame based on objects under comparison
    Vec3 origin = relative_to_sector({0, 0, 0}, *position_rf, transform->position);

    Vec3 bbox_points[] = {
        {bbox->x1, bbox->y1, bbox->z1},
        {bbox->x1, bbox->y1, bbox->z2},
        {bbox->x1, bbox->y2, bbox->z1},
        {bbox->x1, bbox->y2, bbox->z2},
        {bbox->x2, bbox->y1, bbox->z1},
        {bbox->x2, bbox->y1, bbox->z2},
        {bbox->x2, bbox->y2, bbox->z1},
        {bbox->x2, bbox->y2, bbox->z2},
    };

    Mat33 orientation = transform->orientation.to_matrix();

    /* TODO: We may want some sort of tolerance for checking if the support result
       is an edge or surface. Previously I was averaging the set of points with an
       equal dot product, but floating point == comparisons seemed like a bad idea.
       Should look into this more. */

    Vec3 result;
    float max_dot = -FLT_MAX;
    for (uint32_t i = 0; i < ARRAY_LENGTH(bbox_points); ++i)
    {
        Vec3 position =  orientation * bbox_points[i] + origin;
        float current_dot = dot(*reinterpret_cast<const Vec3*>(dir), position);
        if (current_dot > max_dot)
        {
            max_dot = current_dot;
            result = position;
            // npoints = 1;
        }
        //else if (current_dot == max_dot)
        //{
        //    // running avg
        //    result = (1.0f / (npoints + 1)) * (npoints * result + position);
        //    ++npoints;
        //}
    }

    vec->v[0] = result.x;
    vec->v[1] = result.y;
    vec->v[2] = result.z;
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
