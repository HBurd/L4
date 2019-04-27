#include "hb/projectile.h"
#include "hb/mesh_type.h"

Entity create_projectile(Transform shooter_transform)
{
    Entity result;
    result.supported_components =
        ComponentType::WORLD_SECTOR
        | ComponentType::TRANSFORM
        | ComponentType::PHYSICS
        | ComponentType::MESH
        | ComponentType::PROJECTILE;
    result.transform = shooter_transform;
    result.transform.velocity += 0.7f * (shooter_transform.orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f));
    result.transform.angular_velocity;
    result.mesh = MeshType::PROJECTILE;

    // push projectile forward so it doesn't collide with shooter
    result.transform.position += 2 * result.transform.velocity;

    return result;
}

bool projectile_update(Projectile* projectile)
{
    projectile->timeout--;
    return projectile->timeout ? true : false;
}
