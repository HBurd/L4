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

    EntityHandle handle = entity_manager->create_entity(
        required_components,
        ARRAY_LENGTH(required_components));
    
    EntityRef ref;
    entity_manager->entity_table.lookup_entity(handle, &ref);

    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;

    Transform *transform_cmp = new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    transform_cmp->scale = radius * Vec3(1.0f, 1.0f, 1.0f);

    new (entity_manager->lookup_component(ref, ComponentType::PLANET)) Planet(radius, mass);

    MeshId *mesh_cmp = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh_cmp = MeshType::PLANET;
}
