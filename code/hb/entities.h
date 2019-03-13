#ifndef HBENTITIES_H
#define HBENTITIES_H

#include <vector>
#include "hb/TransformComponent.h"
#include "hb/renderer.h"

using std::vector;

const size_t MAX_ENTITIES = 65536;

namespace ComponentType
{
    enum ComponentType
    {
        PHYSICS = 1,
        MESH = 2,               // TODO: can we include component dependencies in here?
        PLAYER_CONTROL = 4,
        PROJECTILE = 8,
        TRANSFORM = 16,
        PLANET = 32,

        // =======================================
        // Add components here as they are created
        // =======================================
    };
}

struct Planet
{
    float radius = 1.0f;
    float mass = 1.0f;

    Planet() = default;
    Planet(float planet_radius, float planet_mass);
};

struct Physics
{
    float mass = 1.0f;
};

// TODO: this doesn't belong here
typedef size_t ClientId;

struct PlayerControl
{
    ClientId client_id;
};

struct Projectile
{
    unsigned int timeout = 60; // 1 second
};

// Used for initializing the components of new entities
// Values are copied into new components
struct Entity
{
    uint32_t supported_components = 0;   // bitfield containing implemented components

    Transform transform;
    Physics physics;
    MeshId mesh_id;
    PlayerControl player_control;
    Projectile projectile;
    Planet planet;

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
    vector<Transform> transform_list;
    vector<Physics> physics_list;
    vector<MeshId> mesh_list;
    vector<PlayerControl> player_control_list;
    vector<Projectile> projectile_list;
    vector<Planet> planet_list;

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
