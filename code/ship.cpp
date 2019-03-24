#include "hb/ship.h"
#include "hb/mesh_type.h"

Entity create_ship(Vec3 position)
{
    Entity result;
    result.supported_components =
        ComponentType::TRANSFORM | ComponentType::PHYSICS | ComponentType::MESH;
    result.transform = Transform(position);
    result.physics.mass = 1.0f;
    result.physics.angular_mass = 1.0f;
    result.mesh_id = MeshType::SHIP;

    return result;
}
