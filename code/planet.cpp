#include "hb/planet.h"
#include "hb/mesh_type.h"

void create_planet(Vec3 position, float radius, float mass, EntityManager *entity_manager)
{
    uint32_t required_components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::MESH,
        ComponentType::PLANET
    };

    EntityRef ref = entity_manager->create_entity(
        required_components,
        ARRAY_LENGTH(required_components));
    
    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;

    Transform *transform_cmp = new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    transform_cmp->scale = radius * Vec3(1.0f, 1.0f, 1.0f);

    new (entity_manager->lookup_component(ref, ComponentType::PLANET)) Planet(radius, mass);

    MeshId *mesh_cmp = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh_cmp = MeshType::PLANET;
}
