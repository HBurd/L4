#ifndef HBENTITIES_H
#define HBENTITIES_H

#include <vector>
#include "hbrenderer.h"

using std::vector;

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

// Used for initializing the components of new entities
// Values are copied into new components
struct Entity
{
    uint32_t supported_components;   // bitfield containing implemented components
    Physics physics;
    MeshId mesh_id;

    // =======================================
    // Add components here as they are created
    // =======================================
};

struct EntityList
{
    EntityList(uint32_t _supported_components);
    void add_entity(Entity entity);
    bool supports_components(uint32_t components) const;

    size_t size;
    uint32_t supported_components;   // bitfield containing implemented components
    vector<Physics> physics_list;
    vector<MeshId> mesh_list;

    // =======================================
    // Add components here as they are created
    // =======================================
};

struct EntityHandle
{
    uint32_t version;
    size_t idx;
};

struct EntityTableEntry
{
    uint32_t version;
    size_t list_idx;
    size_t entity_idx;
};

struct EntityTable
{
    vector<EntityTableEntry> entries;

    EntityHandle add_entry(size_t list_idx, size_t entity_idx);
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
};

#endif // include guard
