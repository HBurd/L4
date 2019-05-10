#ifndef HBENTITIES_H
#define HBENTITIES_H

#include <vector>
#include "hb/TransformComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/ProjectileComponent.h"
#include "hb/PlanetComponent.h"
#include "hb/MeshComponent.h"

const uint32_t MAX_ENTITIES = 65536;
const uint32_t COMPONENT_LIST_SIZE_INCREMENT = 256;

namespace ComponentType
{
    enum ComponentType
    {
        PHYSICS,
        MESH,
        PLAYER_CONTROL,
        PROJECTILE,
        TRANSFORM,
        PLANET,
        WORLD_SECTOR,

        // =======================================
        // Add components here as they are created
        // =======================================
        
        NUM_COMPONENT_TYPES
    };
}

struct EntityRef
{
    uint32_t list_idx;
    uint32_t entity_idx;
};

struct EntityHandle
{
    int32_t version = 0;  // 0 for uninitialized
    uint32_t idx;

    bool is_initialized() const;
};

bool operator==(const EntityHandle& lhs, const EntityHandle& rhs);

struct EntityTableEntry
{
    int32_t version;
    EntityRef entity_ref;
};

struct EntityTable
{
    uint8_t used_entries[(MAX_ENTITIES + 7) / 8] = {}; // rounding up to nearest 8
    EntityTableEntry entries[MAX_ENTITIES] = {};

    EntityHandle pick_handle();

    EntityHandle add_entry(EntityRef entity_ref);
    void add_entry_with_handle(EntityRef entity_ref, EntityHandle handle);
    bool lookup_entity(
        EntityHandle handle,
        EntityRef *entity_ref) const;

    void free_handle(EntityHandle handle);
    void update_handle(EntityHandle handle, EntityRef new_entity_ref);
};

struct EntityListInfo
{
    uint32_t size = 0;
    uint32_t max_size;
    EntityHandle *handles = nullptr;
    void *components[ComponentType::NUM_COMPONENT_TYPES] = {};

    bool supports_component(uint32_t component_type) const;
};

struct ComponentWrapper
{
    uint32_t type;
    uint32_t size;      // of data
    uint8_t data[];
};

struct ComponentInfo
{
    uint32_t size;
};

struct EntityManager
{
    EntityManager(
        ComponentInfo *components,
        uint32_t num_components_,
        uint8_t *component_data_,
        size_t size);

    ComponentInfo *component_info;
    uint32_t num_components;

    std::vector<EntityListInfo> entity_lists;
    EntityTable entity_table;
    uint8_t *component_data;
    size_t component_data_size;
    size_t component_data_used = 0;

    EntityHandle create_entity(
        uint32_t *required_components,
        uint32_t num_required_components);

    void create_entity_with_handle(
        uint32_t *required_components,
        uint32_t num_required_components,
        EntityHandle handle);
    void create_entity_from_serialized(
        uint8_t *entity_data,
        size_t data_size,
        EntityHandle handle);

    void serialize_entity(
        EntityRef entity_ref,
        uint8_t *data,
        size_t *size);
    size_t serialize_entity_size(EntityRef entity_ref);

    void kill_entity(EntityHandle handle);
    void *lookup_component(
        EntityRef entity_ref,
        uint32_t cmp_type) const;
};

#ifdef FAST_BUILD
#include "entities.cpp"
#endif

#endif // include guard
