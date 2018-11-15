#include "hbentities.h"
#include <cassert>

Physics::Physics(Vec3 _position)
:position(_position) {}

EntityList::EntityList(uint32_t _supported_components)
:supported_components(_supported_components) {}

void EntityList::add_entity(Entity entity)
{
    assert(entity.supported_components == supported_components);
    size++;

    if (supported_components & ComponentType::PHYSICS)
    {
        physics_list.push_back(entity.physics);
        assert(physics_list.size() == size);
    }
    if (supported_components & ComponentType::MESH)
    {
        mesh_list.push_back(entity.mesh_id);
        assert(mesh_list.size() == size);
    }
}

bool EntityList::supports_components(uint32_t components) const
{
    return (supported_components & components) == components;
}

EntityHandle EntityTable::add_entry(size_t list_idx, size_t entity_idx)
{
    // TODO: temporary naive implementation
    EntityHandle handle = {.version = 0, .idx = entries.size()};
    EntityTableEntry new_entry = {.version = 0, .list_idx = list_idx, .entity_idx = entity_idx};
    entries.push_back(new_entry);
    return handle;
}

bool EntityTable::lookup_entity(
    EntityHandle handle,
    const vector<EntityList>& entity_lists,
    size_t* list_idx,
    size_t* entity_idx)
{
    if (handle.version != entries[handle.idx].version)
    {
        return false;
    }

    *list_idx = entries[handle.idx].list_idx;
    *entity_idx = entries[handle.idx].entity_idx;
    
    return true;
}

EntityHandle EntityManager::create_entity(Entity entity)
{
    // find a suitable EntityList for this entity
    for (size_t list_idx = 0; list_idx < entity_lists.size(); list_idx++)
    {
        EntityList& entity_list = entity_lists[list_idx];

        // components must match exactly
        if (entity_list.supported_components == entity.supported_components)
        {
            size_t entity_idx = entity_list.size;
            entity_list.add_entity(entity);
            return entity_table.add_entry(list_idx, entity_idx);
        }
    }

    assert(false);  // unable to find suitable list
    return {};
}
