#include "hb/entity_initializers.h"
#include "hb/util.h"
#include "hb/renderer.h"
#include "hb/components.h"
#include "hb/PlanetComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/ProjectileComponent.h"

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

EntityRef create_ship(Vec3 position, EntityManager *entity_manager)
{
    uint32_t components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::PHYSICS,
        ComponentType::MESH
    };
    EntityRef ref = entity_manager->create_entity(components, ARRAY_LENGTH(components));
    
    // Initialize components
    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;
    new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    new (entity_manager->lookup_component(ref, ComponentType::PHYSICS)) Physics;
    MeshId *mesh = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh = MeshType::SHIP;

    return ref;
}

EntityRef create_player_ship(Vec3 position, ClientId player, EntityManager *entity_manager)
{
    uint32_t components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::PHYSICS,
        ComponentType::MESH,
        ComponentType::PLAYER_CONTROL
    };
    EntityRef ref = entity_manager->create_entity(components, ARRAY_LENGTH(components));
    
    // Initialize components
    new (entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR)) WorldSector;
    new (entity_manager->lookup_component(ref, ComponentType::TRANSFORM)) Transform(position);
    new (entity_manager->lookup_component(ref, ComponentType::PHYSICS)) Physics;
    MeshId *mesh = new (entity_manager->lookup_component(ref, ComponentType::MESH)) MeshId;
    *mesh = MeshType::SHIP;
    PlayerControl *player_control = new (entity_manager->lookup_component(ref, ComponentType::PLAYER_CONTROL)) PlayerControl;
    player_control->client_id = player;


    return ref;
}

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

