#include "hb/entities.h"
#include "hb/util.h"
#include <cassert>

Physics::Physics(Vec3 _position)
:position(_position) {}

EntityList::EntityList(uint32_t _supported_components)
:supported_components(_supported_components) {}

void EntityList::add_entity(Entity entity, EntityHandle handle)
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
    if (supported_components & ComponentType::PLAYER_CONTROL)
    {
        player_control_list.push_back(entity.player_control);
        assert(player_control_list.size() == size);
    }
    if (supported_components & ComponentType::PROJECTILE)
    {
        projectile_list.push_back(entity.projectile);
        assert(projectile_list.size() == size);
    }
    
    handles.push_back(handle);
    
    assert(handles.size() == size);
}

Entity EntityList::serialize(size_t entity_idx)
{
    Entity entity;
    // we aren't setting entity's supported components directly
    // so that we can test at the end that everything's been added
    if (supported_components & ComponentType::PHYSICS)
    {
        entity.supported_components |= ComponentType::PHYSICS;
        entity.physics = physics_list[entity_idx];
    }
    if (supported_components & ComponentType::MESH)
    {
        entity.supported_components |= ComponentType::MESH;
        entity.mesh_id = mesh_list[entity_idx];
    }
    if (supported_components & ComponentType::PLAYER_CONTROL)
    {
        entity.supported_components |= ComponentType::PLAYER_CONTROL;
        entity.player_control = player_control_list[entity_idx];
    }
    if (supported_components & ComponentType::PROJECTILE)
    {
        entity.supported_components |= ComponentType::PROJECTILE;
        entity.projectile = projectile_list[entity_idx];
    }

    // see above
    assert(entity.supported_components == supported_components);
    return entity;
}

bool EntityList::supports_components(uint32_t components) const
{
    return (supported_components & components) == components;
}

bool EntityHandle::is_initialized() const
{
    return version != 0;
}

bool operator==(const EntityHandle& lhs, const EntityHandle& rhs)
{
    return lhs.version == rhs.version && lhs.idx == rhs.idx;
}

EntityTable::EntityTable()
{
    first_free = 0;
    for (size_t i = 0; i < ARRAY_LENGTH(entries); i++)
    {
        entries[i].next_free = i + 1;
        entries[i].version = -1;
    }
}

EntityHandle EntityTable::add_entry(size_t list_idx, size_t entity_idx)
{
    size_t new_idx = first_free;
    assert(new_idx < ARRAY_LENGTH(entries));

    first_free = entries[first_free].next_free;
    
    assert(entries[new_idx].version < 0);

    entries[new_idx].version = -entries[new_idx].version + 1;
    entries[new_idx].list_idx = list_idx;
    entries[new_idx].entity_idx = entity_idx;
	EntityHandle result;
	result.version = entries[new_idx].version;
	result.idx = new_idx;
	return result;
}

void EntityTable::add_entry_with_handle(size_t list_idx, size_t entity_idx, EntityHandle handle)
{
    // check if the handle's free
    int idx = first_free;
    if (idx == handle.idx)
    {
        first_free = entries[first_free].next_free;

        // check the version corresponds to inactive entry
        assert(entries[handle.idx].version <= 0);

        entries[handle.idx].version = -entries[handle.idx].version + 1;
        entries[handle.idx].list_idx = list_idx;
        entries[handle.idx].entity_idx = entity_idx;
        return;
    }
    while (idx <= ARRAY_LENGTH(entries))
    {
        if (entries[idx].next_free == handle.idx)
        {
            entries[idx].next_free = entries[handle.idx].next_free;

            // check the version corresponds to inactive entry
            // if it doesn't something has gone wrong
            assert(entries[handle.idx].version <= 0);

            entries[handle.idx].version = -entries[handle.idx].version + 1;
            entries[handle.idx].list_idx = list_idx;
            entries[handle.idx].entity_idx = entity_idx;
            return;
        }
        idx = entries[idx].next_free;
    }

    // this means the entry for this handle is already used
    assert(false);
}

bool EntityTable::lookup_entity(
    EntityHandle handle,
    const vector<EntityList>& entity_lists,
    size_t* list_idx,
    size_t* entity_idx) const
{
    if (handle.version == 0 || handle.version != entries[handle.idx].version)
    {
        return false;
    }

    *list_idx = entries[handle.idx].list_idx;
    *entity_idx = entries[handle.idx].entity_idx;

    return true;
}

void EntityTable::free_handle(EntityHandle handle)
{
    if (handle.version != entries[handle.idx].version)
    {
        // this handle has already been freed
        return;
    }
    
    // swap version sign to indicate handle freed
    entries[handle.idx].version = -entries[handle.idx].version;
}

void EntityTable::update_handle(EntityHandle handle, size_t new_list_idx, size_t new_entity_idx)
{
    assert(handle.version == entries[handle.idx].version);
    entries[handle.idx].list_idx = new_list_idx;
    entries[handle.idx].entity_idx = new_entity_idx;
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
            EntityHandle handle = entity_table.add_entry(list_idx, entity_idx);
            entity_list.add_entity(entity, handle);
            return handle;
        }
    }

    assert(false);  // unable to find suitable list
    return {};
}

void EntityManager::create_entity_with_handle(Entity entity, EntityHandle entity_handle)
{
    // find a suitable EntityList for this entity
    for (size_t list_idx = 0; list_idx < entity_lists.size(); list_idx++)
    {
        EntityList& entity_list = entity_lists[list_idx];

        // components must match exactly
        if (entity_list.supported_components == entity.supported_components)
        {
            size_t entity_idx = entity_list.size;
            entity_table.add_entry_with_handle(list_idx, entity_idx, entity_handle);
            entity_list.add_entity(entity, entity_handle);
            return;
        }
    }

    assert(false);  // unable to find suitable list
}

void EntityManager::kill_entity(EntityHandle handle)
{
    // cache list_idx and entity_idx
    size_t list_idx;
    size_t entity_idx;
    entity_table.lookup_entity(
        handle,
        entity_lists,
        &list_idx,
        &entity_idx);

    EntityList& entity_list = entity_lists[list_idx];

    // invalidate handle
    entity_table.free_handle(handle);

    // We are replacing this entity with the last entity in the list
    // so we must update its handle
    if (entity_idx == entity_list.size - 1)
    {
        entity_list.handles.pop_back();
    }
    else
    {
        entity_list.handles[entity_idx] = entity_list.handles.back();
        entity_list.handles.pop_back();
        if (entity_list.handles.size() >  0)
        {
            entity_table.update_handle(entity_list.handles[entity_idx], list_idx, entity_idx);
        }
    }

    // Now remove actual components
    // we have to modify each list individually
    uint32_t removed_components = 0;
    if (entity_list.supports_components(ComponentType::PHYSICS))
    {
        entity_list.physics_list[entity_idx] = entity_list.physics_list.back();
        entity_list.physics_list.pop_back();
        removed_components |= ComponentType::PHYSICS;
    }
    if (entity_list.supports_components(ComponentType::MESH))
    {
        entity_list.mesh_list[entity_idx] = entity_list.mesh_list.back();
        entity_list.mesh_list.pop_back();
        removed_components |= ComponentType::MESH;
    }
    if (entity_list.supports_components(ComponentType::PLAYER_CONTROL))
    {
        entity_list.player_control_list[entity_idx] = entity_list.player_control_list.back();
        entity_list.player_control_list.pop_back();
        removed_components |= ComponentType::PLAYER_CONTROL;
    }
    if (entity_list.supports_components(ComponentType::PROJECTILE))
    {
        entity_list.projectile_list[entity_idx] = entity_list.projectile_list.back();
        entity_list.projectile_list.pop_back();
        removed_components |= ComponentType::PROJECTILE;
    }

    // verify we removed all of this entity's components
    assert(removed_components == entity_list.supported_components);
    entity_list.size--;
}
