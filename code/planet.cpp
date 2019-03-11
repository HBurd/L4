#include "hb/planet.h"
#include "hb/mesh_type.h"

Entity create_planet(Vec3 position, float radius)
{
    Entity result;
    result.supported_components = ComponentType::TRANSFORM | ComponentType::MESH;
    result.transform= Transform(position);
    result.transform.scale = radius * Vec3(1.0f, 1.0f, 1.0f);
    result.mesh_id = MeshType::PLANET;

    return result;
}
