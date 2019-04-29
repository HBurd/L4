#include "hb/entities.h"
#include "hb/util.h"
#include <cassert>

EntityList::EntityList(uint32_t _supported_components)
:supported_components(_supported_components) {}

#define ADD_COMPONENT(C) EXPAND_MACRO(ADD_COMPONENT2(C))
#define ADD_COMPONENT2(type, name, id) { \
    if (supports_components(ComponentType::id)) \
    { \
        name##_list.push_back(entity.name); \
        assert(name##_list.size() == size); \
    } \
}

void EntityList::add_entity(Entity entity, EntityHandle handle)
{
    assert(entity.supported_components == supported_components);
    size++;

    ADD_COMPONENT(WORLD_SECTOR_COMPONENT);
    ADD_COMPONENT(TRANSFORM_COMPONENT);
    ADD_COMPONENT(PHYSICS_COMPONENT);
    ADD_COMPONENT(MESH_COMPONENT);
    ADD_COMPONENT(PLAYER_CONTROL_COMPONENT);
    ADD_COMPONENT(PROJECTILE_COMPONENT);
    ADD_COMPONENT(PLANET_COMPONENT);
    
    handles.push_back(handle);
    
    assert(handles.size() == size);
}

#define SERIALIZE_COMPONENT(C) EXPAND_MACRO(SERIALIZE_COMPONENT2(C))
#define SERIALIZE_COMPONENT2(type, name, id) { \
    if (supports_components(ComponentType::id)) \
    { \
        entity.supported_components |= ComponentType::id; \
        entity.name = name##_list[entity_idx]; \
    } \
}

Entity EntityList::serialize(size_t entity_idx)
{
    Entity entity;
    // we aren't setting entity's supported components directly (it is set in macro)
    // so that we can test at the end that everything's been added
    SERIALIZE_COMPONENT(WORLD_SECTOR_COMPONENT);
    SERIALIZE_COMPONENT(TRANSFORM_COMPONENT);
    SERIALIZE_COMPONENT(PHYSICS_COMPONENT);
    SERIALIZE_COMPONENT(MESH_COMPONENT);
    SERIALIZE_COMPONENT(PLAYER_CONTROL_COMPONENT);
    SERIALIZE_COMPONENT(PROJECTILE_COMPONENT);
    SERIALIZE_COMPONENT(PLANET_COMPONENT);

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

EntityHandle EntityTable::add_entry(size_t list_idx, size_t entity_idx)
{
    // find a free entry
    uint32_t i;
    for (i = 0; i < ARRAY_LENGTH(used_entries); i++)
    {
        if (used_entries[i] != 0xFF) break;
    }
    
    uint32_t index = i * 8 * sizeof(*used_entries);
    uint8_t used_entries_bits = used_entries[i];
    while(used_entries_bits & 1)
    {
        used_entries_bits >>= 1;
        index++;
    }

    entries[index].version = -entries[index].version + 1;
    entries[index].list_idx = list_idx;
    entries[index].entity_idx = entity_idx;

    EntityHandle new_handle;
    new_handle.version = entries[index].version;
    new_handle.idx = index;

    add_entry_with_handle(list_idx, entity_idx, new_handle);

    return new_handle;
}

void EntityTable::add_entry_with_handle(size_t list_idx, size_t entity_idx, EntityHandle handle)
{
    // mark the entry as used
    uint32_t used_entries_idx = handle.idx / (8 * sizeof(*used_entries));
    uint32_t bit_offset = handle.idx % (8 * sizeof(*used_entries));
    assert((used_entries[used_entries_idx] & 1 << bit_offset) == 0);
    used_entries[used_entries_idx] |= 1 << bit_offset;

    // now set the entry
    entries[handle.idx].version = handle.version;
    entries[handle.idx].list_idx = list_idx;
    entries[handle.idx].entity_idx = entity_idx;
}

bool EntityTable::lookup_entity(
    EntityHandle handle,
    const vector<EntityList>& entity_lists,
    EntityListIdx* list_idx,
    EntityIdx* entity_idx) const
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
        cout << "warning, freeing handle multiple times" << endl;
        return;
    }

    uint32_t used_entries_idx = handle.idx / (8 * sizeof(*used_entries));
    uint32_t bit_offset = handle.idx % (8 * sizeof(*used_entries));

    // swap version sign to indicate handle freed
    entries[handle.idx].version = -entries[handle.idx].version;
    // clear the used bit
    used_entries[used_entries_idx] &= ~(1 << bit_offset);
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
    EntityList *entity_list = nullptr;
    size_t list_idx;
    for (list_idx = 0; list_idx < entity_lists.size(); list_idx++)
    {
        // components must match exactly
        if (entity_lists[list_idx].supported_components == entity.supported_components)
        {
            entity_list = &entity_lists[list_idx];
            break;
        }
    }

    // make a new list if a suitable one wasn't found
    if (entity_list == nullptr)
    {
        list_idx = entity_lists.size();
        entity_lists.push_back(EntityList(entity.supported_components));
        entity_list = &entity_lists.back();
    }

    size_t entity_idx = entity_list->size;
    EntityHandle handle = entity_table.add_entry(list_idx, entity_idx);
    entity_list->add_entity(entity, handle);
    return handle;
}

void EntityManager::create_entity_with_handle(Entity entity, EntityHandle entity_handle)
{
    // find a suitable EntityList for this entity
    EntityList *entity_list = nullptr;
    size_t list_idx;
    for (list_idx = 0; list_idx < entity_lists.size(); list_idx++)
    {
        // components must match exactly
        if (entity_lists[list_idx].supported_components == entity.supported_components)
        {
            entity_list = &entity_lists[list_idx];
            break;
        }
    }

    // make a new list if a suitable one wasn't found
    if (entity_list == nullptr)
    {
        list_idx = entity_lists.size();
        entity_lists.push_back(EntityList(entity.supported_components));
        entity_list = &entity_lists.back();
    }

    size_t entity_idx = entity_list->size;
    entity_table.add_entry_with_handle(list_idx, entity_idx, entity_handle);
    entity_list->add_entity(entity, entity_handle);
    return;
}

#define REMOVE_COMPONENT(C) EXPAND_MACRO(REMOVE_COMPONENT2(C))
#define REMOVE_COMPONENT2(type, name, id) {\
    if (entity_list.supports_components(ComponentType::id)) \
    { \
        entity_list.name##_list[entity_idx] = entity_list.name##_list.back(); \
        entity_list.name##_list.pop_back(); \
        removed_components |= ComponentType::id; \
    } \
}

void EntityManager::kill_entity(EntityHandle handle)
{
    // cache list_idx and entity_idx
    EntityListIdx list_idx;
    EntityIdx entity_idx;
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
    REMOVE_COMPONENT(WORLD_SECTOR_COMPONENT);
    REMOVE_COMPONENT(TRANSFORM_COMPONENT);
    REMOVE_COMPONENT(PHYSICS_COMPONENT);
    REMOVE_COMPONENT(MESH_COMPONENT);
    REMOVE_COMPONENT(PLAYER_CONTROL_COMPONENT);
    REMOVE_COMPONENT(PROJECTILE_COMPONENT);
    REMOVE_COMPONENT(PLANET_COMPONENT);

    // verify we removed all of this entity's components
    assert(removed_components == entity_list.supported_components);
    entity_list.size--;
}
