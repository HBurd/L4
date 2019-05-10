#include "hb/entities.h"
#include "hb/util.h"
#include <cassert>

bool EntityListInfo::supports_component(uint32_t component_type) const
{
    return components[component_type] != nullptr;
}

void EntityManager::serialize_entity(
    EntityRef ref,
    uint8_t *data,
    size_t *size)
{
    size_t used_size = 0;
    for (uint32_t i = 0; i < num_components; i++)
    {
        if (entity_lists[ref.list_idx].components[i] != nullptr)
        {
            ComponentWrapper *wrapper = (ComponentWrapper*)(data + used_size);
            size_t component_entry_size = sizeof(ComponentWrapper) + component_info[i].size;
            used_size += component_entry_size;
            assert(used_size <= *size);
            wrapper->type = i;
            wrapper->size = component_info[i].size;
            memcpy(
                wrapper->data,
                lookup_component(ref, i),
                component_info[i].size);
        }
    }
    *size = used_size;
}

void EntityManager::create_entity_from_serialized(
    uint8_t *entity_data,
    size_t data_size,
    EntityHandle handle)
{
    // find what components are required
    uint32_t required_components[ComponentType::NUM_COMPONENT_TYPES];
    uint32_t num_required_components = 0;

    size_t read_size = 0;
    while (read_size < data_size)
    {
        ComponentWrapper *wrapper = (ComponentWrapper*)(entity_data + read_size);
        required_components[num_required_components] = wrapper->type;
        num_required_components++;
        read_size += wrapper->size + sizeof(ComponentWrapper);
    }

    create_entity_with_handle(required_components, num_required_components, handle);

    EntityRef ref;
    entity_table.lookup_entity(handle, &ref);

    // copy in the component data
    read_size = 0;
    for (uint32_t i = 0; i < num_required_components; i++)
    {
        ComponentWrapper *wrapper = (ComponentWrapper*)(entity_data + read_size);
        memcpy(
            lookup_component(ref, required_components[i]),
            wrapper->data,
            wrapper->size);
        read_size += wrapper->size + sizeof(ComponentWrapper);
    }
}

size_t EntityManager::serialize_entity_size(EntityRef ref)
{
    size_t size = 0;
    for (uint32_t i = 0; i < num_components; i++)
    {
        if (entity_lists[ref.list_idx].components[i] != nullptr)
        {
            size_t component_entry_size = sizeof(ComponentWrapper) + component_info[i].size;
            size += component_entry_size;
        }
    }
    return size;
}

bool EntityHandle::is_initialized() const
{
    return version != 0;
}

bool operator==(const EntityHandle &lhs, const EntityHandle &rhs)
{
    return lhs.version == rhs.version && lhs.idx == rhs.idx;
}

EntityHandle EntityTable::pick_handle()
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

    EntityHandle handle;
    handle.version = -entries[index].version + 1;
    handle.idx = index;

    return handle;
}

EntityHandle EntityTable::add_entry(EntityRef ref)
{
    EntityHandle new_handle = pick_handle();
    add_entry_with_handle(ref, new_handle);
    return new_handle;
}

void EntityTable::add_entry_with_handle(EntityRef ref, EntityHandle handle)
{
    // mark the entry as used
    uint32_t used_entries_idx = handle.idx / (8 * sizeof(*used_entries));
    uint32_t bit_offset = handle.idx % (8 * sizeof(*used_entries));
    assert((used_entries[used_entries_idx] & 1 << bit_offset) == 0);
    used_entries[used_entries_idx] |= 1 << bit_offset;

    // now set the entry
    entries[handle.idx].version = handle.version;
    entries[handle.idx].entity_ref = ref;
}

bool EntityTable::lookup_entity(
    EntityHandle handle,
    EntityRef *ref) const
{
    if (handle.version == 0 || handle.version != entries[handle.idx].version)
    {
        return false;
    }

    *ref = entries[handle.idx].entity_ref;

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

void EntityTable::update_handle(EntityHandle handle, EntityRef new_ref)
{
    assert(handle.version == entries[handle.idx].version);
    entries[handle.idx].entity_ref = new_ref;
}

EntityManager::EntityManager(
    ComponentInfo *components,
    uint32_t num_components_,
    uint8_t *component_data_,
    size_t size)
{
    component_info = components;
    num_components = num_components_;
    component_data = component_data_;
    component_data_size = size;
}

EntityHandle EntityManager::create_entity(
    uint32_t *required_components,
    uint32_t num_required_components)
{
    EntityHandle handle = entity_table.pick_handle();
    create_entity_with_handle(
        required_components,
        num_required_components,
        handle);
    return handle;
}

void EntityManager::create_entity_with_handle(
    uint32_t *required_components,
    uint32_t num_required_components,
    EntityHandle handle)
{
    uint32_t li;
    bool found_list = false;
    for(li = 0; li < entity_lists.size(); li++)
    {
        // Unfortunately this is a little complicated, since
        // it's not sufficient to find a list that supports a
        // superset of the components required by the entity;
        // it must also not support any extras. So for now
        // we'll just do this the simplest way possible.
        uint32_t list_num_components = 0;
        for (uint32_t lci = 0;
             lci < num_components;
             lci++)
        {
            if (entity_lists[li].components[lci] != nullptr)
            {
                list_num_components++;
            }
        }
        if (list_num_components == num_required_components
            && entity_lists[li].size < entity_lists[li].max_size)
        {
            found_list = true;
            for (uint32_t eci = 0;
                 eci < num_required_components;
                 eci++)
            {
                if (entity_lists[li].components[required_components[eci]] == nullptr)
                {
                    found_list = false;
                    break;
                }
            }
            if (found_list) break;
        }
    }
    // make a new list if a suitable one wasn't found
    if (!found_list)
    {
        EntityListInfo list_info;
        list_info.max_size = COMPONENT_LIST_SIZE_INCREMENT;

        for (uint32_t i = 0; i < num_required_components; i++)
        {
            list_info.components[required_components[i]] = component_data + component_data_used;
            component_data_used += COMPONENT_LIST_SIZE_INCREMENT * component_info[required_components[i]].size;
        }
        list_info.handles = (EntityHandle*)(component_data + component_data_used);
        component_data_used += COMPONENT_LIST_SIZE_INCREMENT * sizeof(EntityHandle);

        assert(component_data_used <= component_data_size);

        li = entity_lists.size();
        entity_lists.push_back(list_info);
    }

    uint32_t entity_idx = entity_lists[li].size;

    entity_lists[li].handles[entity_idx] = handle;

    EntityRef ref;
    ref.entity_idx = entity_idx;
    ref.list_idx = li;

    entity_table.add_entry_with_handle(ref, handle);
    entity_lists[li].size++;
}

void EntityManager::kill_entity(EntityHandle handle)
{
    // cache entity reference
    EntityRef ref;
    // TODO: Check if we actually found the entity
    entity_table.lookup_entity(
        handle,
        &ref);

    // invalidate handle
    entity_table.free_handle(handle);

    EntityListInfo &entity_list = entity_lists[ref.list_idx];

    EntityRef back;
    back.list_idx = ref.list_idx;
    back.entity_idx = entity_list.size - 1;

    if (ref.entity_idx != back.entity_idx)
    {
        entity_list.handles[ref.entity_idx] = entity_list.handles[back.entity_idx];
        entity_table.update_handle(entity_list.handles[ref.entity_idx], ref);

        for (uint32_t i = 0; i < num_components; i++)
        {
            if (entity_list.components[i])
            {
                memcpy(
                    lookup_component(ref, i),
                    lookup_component(back, i),
                    component_info[i].size);
            }
        }
    }

    entity_list.size--;
}

void *EntityManager::lookup_component(
    EntityRef ref,
    uint32_t cmp_type) const
{
    return (uint8_t*)(entity_lists[ref.list_idx].components[cmp_type]) + ref.entity_idx * component_info[cmp_type].size;
}
