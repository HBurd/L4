#ifndef HBENTITIES_H
#define HBENTITIES_H

#include <vector>
#include "hb/TransformComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/ProjectileComponent.h"
#include "hb/PlanetComponent.h"
#include "hb/MeshComponent.h"

using std::vector;

const size_t MAX_ENTITIES = 65536;

#define COMPONENT_ID(C) COMPONENT_ID2(C)
#define COMPONENT_ID2(type, name, id) id

#define COMPONENT_DECLARATION(C) COMPONENT_DECLARATION2(C)
#define COMPONENT_DECLARATION2(type, name, id) type name

#define COMPONENT_LIST_DECLARATION(C) COMPONENT_LIST_DECLARATION2(C)
#define COMPONENT_LIST_DECLARATION2(type, name, id) std::vector<type> name##_list

namespace ComponentType
{
    enum ComponentType
    {
        // TODO: can we include component dependencies in here?
        COMPONENT_ID(PHYSICS_COMPONENT) = 1,
        COMPONENT_ID(MESH_COMPONENT) = 2,
        COMPONENT_ID(PLAYER_CONTROL_COMPONENT) = 4,
        COMPONENT_ID(PROJECTILE_COMPONENT) = 8,
        COMPONENT_ID(TRANSFORM_COMPONENT) = 16,
        COMPONENT_ID(PLANET_COMPONENT) = 32,
        COMPONENT_ID(WORLD_SECTOR_COMPONENT) = 64,

        // =======================================
        // Add components here as they are created
        // =======================================
    };
}

// Used for initializing the components of new entities
// Values are copied into new components
struct Entity
{
    uint32_t supported_components = 0;   // bitfield containing implemented components

    COMPONENT_DECLARATION(WORLD_SECTOR_COMPONENT);
    COMPONENT_DECLARATION(TRANSFORM_COMPONENT);
    COMPONENT_DECLARATION(PHYSICS_COMPONENT);
    COMPONENT_DECLARATION(MESH_COMPONENT);
    COMPONENT_DECLARATION(PLAYER_CONTROL_COMPONENT);
    COMPONENT_DECLARATION(PROJECTILE_COMPONENT);
    COMPONENT_DECLARATION(PLANET_COMPONENT);

    // =======================================
    // Add components here as they are created
    // =======================================
};

struct EntityHandle
{
    int32_t version = 0;  // 0 for uninitialized
    size_t idx;

    bool is_initialized() const;
};

bool operator==(const EntityHandle& lhs, const EntityHandle& rhs);

struct EntityList
{
    EntityList(uint32_t _supported_components);
    void add_entity(Entity entity, EntityHandle handle);
    bool supports_components(uint32_t components) const;
    Entity serialize(size_t entity_idx);

    size_t size = 0;
    uint32_t supported_components;   // bitfield containing implemented components

    COMPONENT_LIST_DECLARATION(TRANSFORM_COMPONENT);
    COMPONENT_LIST_DECLARATION(PHYSICS_COMPONENT);
    COMPONENT_LIST_DECLARATION(MESH_COMPONENT);
    COMPONENT_LIST_DECLARATION(PLAYER_CONTROL_COMPONENT);
    COMPONENT_LIST_DECLARATION(PROJECTILE_COMPONENT);
    COMPONENT_LIST_DECLARATION(PLANET_COMPONENT);

    // =======================================
    // Add components here as they are created
    // =======================================
    
    vector<EntityHandle> handles;
};

struct EntityTableEntry
{
    int32_t version;
    union
    {
        struct
        {
            size_t list_idx;
            size_t entity_idx;
        };
        size_t next_free;
    };
};

struct EntityTable
{
    EntityTable();

    size_t first_free;
    EntityTableEntry entries[MAX_ENTITIES] = {};

    EntityHandle add_entry(size_t list_idx, size_t entity_idx);
    void add_entry_with_handle(size_t list_idx, size_t entity_idx, EntityHandle handle);
    bool lookup_entity(
        EntityHandle handle,
        const vector<EntityList>& entity_lists,
        size_t* list_idx,
        size_t* entity_idx) const;

    void free_handle(EntityHandle handle);
    void update_handle(EntityHandle handle, size_t new_list_idx, size_t new_entity_idx);
};

struct EntityManager
{
    vector<EntityList> entity_lists;
    EntityTable entity_table;

    EntityHandle create_entity(Entity entity);
    void create_entity_with_handle(Entity entity, EntityHandle entity_handle);
    
    void kill_entity(EntityHandle handle);
};

#ifdef FAST_BUILD
#include "entities.cpp"
#endif

#endif // include guard
