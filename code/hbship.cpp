#include "hbship.h"

Entity create_ship(Vec3 position, MeshId mesh_id)
{
    Entity result;
    result.supported_components = ComponentType::PHYSICS | ComponentType::MESH;
    result.physics = Physics(position);
    result.mesh_id = mesh_id;

    return result;
}
