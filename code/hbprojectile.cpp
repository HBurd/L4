#include "hbprojectile.h"

Entity create_projectile(Physics shooter_physics, MeshId mesh_id)
{
    Entity result;
    result.supported_components = ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PROJECTILE;
    result.physics = shooter_physics;
    result.physics.velocity += 0.7f * (shooter_physics.orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f));
    result.physics.angular_velocity = Rotor();
    result.mesh_id = mesh_id;

    // push projectile forward so it doesn't collide with shooter
    result.physics.position += 2 * result.physics.velocity;

    return result;
}

bool projectile_update(Projectile* projectile)
{
    projectile->timeout--;
    return projectile->timeout ? true : false;
}
