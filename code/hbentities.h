#ifndef HBENTITIES_H
#define HBENTITIES_H

#include <vector>
#include "hbrenderer.h"

using std::vector;

const size_t MAX_ENTITIES = 65536;

namespace ComponentType
{
    enum ComponentType
    {
        PHYSICS = 1,
        MESH = 2,               // TODO: can we include component dependencies in here?
        PLAYER_CONTROL = 4,

        // =======================================
        // Add components here as they are created
        // =======================================
    };
}

struct Physics
{
    Vec3 position;
    Vec3 velocity;
    Rotor orientation;
    Rotor angular_velocity;

    Physics() = default;
    Physics(Vec3 _position);
};

// TODO: this doesn't belong here
typedef size_t ClientId;

struct PlayerControl
{
    ClientId client_id;
};

// Used for initializing the components of new entities
// Values are copied into new components
struct Entity
{
    uint32_t supported_components;   // bitfield containing implemented components

    Physics physics;
    MeshId mesh_id;
    PlayerControl player_control;

    // =======================================
    // Add components here as they are created
    // =======================================
};

struct EntityHandle
{
    int32_t version;
    size_t idx;
};

struct EntityList
{
    EntityList(uint32_t _supported_components);
    void add_entity(Entity entity, EntityHandle handle);
    bool supports_components(uint32_t components) const;
    Entity serialize(size_t entity_idx);

    size_t size = 0;
    uint32_t supported_components;   // bitfield containing implemented components
    vector<Physics> physics_list;
    vector<MeshId> mesh_list;
    vector<PlayerControl> player_control_list;

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
        size_t* entity_idx);
};

struct EntityManager
{
    vector<EntityList> entity_lists;
    EntityTable entity_table;

    EntityHandle create_entity(Entity entity);
    void create_entity_with_handle(Entity entity, EntityHandle entity_handle);
};

#endif // include guard
