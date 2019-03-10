#include "hb/planet.h"
#include "hb/mesh_type.h"

Entity create_planet(Vec3 position, float radius)
{
    Entity result;
    result.supported_components = ComponentType::PHYSICS | ComponentType::MESH;
    result.physics = Physics(position);
    result.mesh_id = MeshType::PLANET;

    return result;
}
