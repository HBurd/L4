#include "hb/ship.h"
#include "hb/mesh_type.h"

EntityHandle create_ship(Vec3 position, EntityManager *entity_manager)
{
    uint32_t components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::PHYSICS,
        ComponentType::MESH
    };
    EntityHandle handle = entity_manager->create_entity(components, ARRAY_LENGTH(components));
    EntityRef ref;
    entity_manager->entity_table.lookup_entity(handle, &ref);
    
    // Initialize components
    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;
    new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    new (entity_manager->lookup_component(ref, ComponentType::PHYSICS)) Physics;
    MeshId *mesh = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh = MeshType::SHIP;

    return handle;
}

EntityHandle create_player_ship(Vec3 position, ClientId player, EntityManager *entity_manager)
{
    uint32_t components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::PHYSICS,
        ComponentType::MESH,
        ComponentType::PLAYER_CONTROL
    };
    EntityHandle handle = entity_manager->create_entity(components, ARRAY_LENGTH(components));
    EntityRef ref;
    entity_manager->entity_table.lookup_entity(handle, &ref);
    
    // Initialize components
    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;
    new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    new (entity_manager->lookup_component(ref, ComponentType::PHYSICS)) Physics;
    MeshId *mesh = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh = MeshType::SHIP;
    PlayerControl *player_control = new (entity_manager->lookup_component(ref, ComponentType::PLAYER_CONTROL)) PlayerControl;
    player_control->client_id = player;


    return handle;
}
