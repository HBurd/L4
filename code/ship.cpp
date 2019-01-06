#include "hb/ship.h"
#include "hb/mesh_type.h"

Entity create_ship(Vec3 position)
{
    Entity result;
    result.supported_components = ComponentType::PHYSICS | ComponentType::MESH;
    result.physics = Physics(position);
    result.mesh_id = MeshType::SHIP;

    return result;
}
