#include "hb/projectile.h"
#include "hb/mesh_type.h"

EntityRef create_projectile(Transform shooter_transform, EntityManager *entity_manager)
{
    uint32_t required_components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::PHYSICS,
        ComponentType::MESH,
        ComponentType::PROJECTILE
    };
    EntityRef ref = entity_manager->create_entity(required_components, ARRAY_LENGTH(required_components));

    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;

    Transform *transform_cmp = new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(shooter_transform);
    transform_cmp->velocity += 0.7f * (shooter_transform.orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f));
    transform_cmp->angular_velocity = Vec3();

    // push projectile forward so it doesn't collide with shooter
    transform_cmp->position += 2 * transform_cmp->velocity;

    new (entity_manager->lookup_component(ref, ComponentType::PHYSICS)) Physics;
    new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId(MeshType::PROJECTILE);
    new (entity_manager->lookup_component(ref, ComponentType::PROJECTILE)) Projectile;

    return ref;
}

bool projectile_update(Projectile* projectile)
{
    projectile->timeout--;
    return projectile->timeout ? true : false;
}
