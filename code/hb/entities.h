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

typedef uint32_t EntityListIdx;
typedef uint32_t EntityIdx;

// This is a workaround for the microsoft compiler,
// which disregards the standards and treats macros that
// expand to a list of arguments as a single argument
#define EXPAND_MACRO(x) x

#define COMPONENT_ID(C) EXPAND_MACRO(COMPONENT_ID2(C))
#define COMPONENT_ID2(type, name, id) id

#define COMPONENT_DECLARATION(C) EXPAND_MACRO(COMPONENT_DECLARATION2(C))
#define COMPONENT_DECLARATION2(type, name, id) type name

#define COMPONENT_LIST_DECLARATION(C) EXPAND_MACRO(COMPONENT_LIST_DECLARATION2(C))
#define COMPONENT_LIST_DECLARATION2(type, name, id) std::vector<type> name##_list

#define LOOKUP_COMPONENT(C, handle, entity_manager, varname) \
    EXPAND_MACRO(LOOKUP_COMPONENT2(C, handle, entity_manager, varname))
#define LOOKUP_COMPONENT2(type, compname, id, handle, entity_manager, declaration) \
    EntityListIdx list_idx; \
    EntityIdx entity_idx; \
    int TMP_##__LINE__ = 1;\
    if ((entity_manager).entity_table.lookup_entity( \
            handle, \
            (entity_manager).entity_lists, \
            &list_idx, \
            &entity_idx)) \
        if ((entity_manager).entity_lists[list_idx].supports_components(ComponentType::id)) \
            for (declaration = \
                    (entity_manager).entity_lists[list_idx].compname##_list[entity_idx]; \
                 TMP_##__LINE__ > 0; \
                 TMP_##__LINE__--)

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

    COMPONENT_LIST_DECLARATION(WORLD_SECTOR_COMPONENT);
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
    size_t list_idx;
    size_t entity_idx;
};

struct EntityTable
{
    uint8_t used_entries[(MAX_ENTITIES + 7) / 8] = {}; // rounding up to nearest 8
    EntityTableEntry entries[MAX_ENTITIES] = {};

    EntityHandle add_entry(size_t list_idx, size_t entity_idx);
    void add_entry_with_handle(size_t list_idx, size_t entity_idx, EntityHandle handle);
    bool lookup_entity(
        EntityHandle handle,
        const vector<EntityList>& entity_lists,
        EntityListIdx* list_idx,
        EntityIdx* entity_idx) const;

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
