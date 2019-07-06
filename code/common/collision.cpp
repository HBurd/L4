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
