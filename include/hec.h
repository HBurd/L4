// Hugo's entity-component helper, v0.2
//
// - Absolutely anything (everything) can (will) change.
// - Absolutely nothing is guaranteed to work.
// - Absolutely nothing is documented.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef HEC_HEADER_INCLUDED
#define HEC_HEADER_INCLUDED

// TODO: Temp hack
#include "hb/components.h"

// TODO: Get rid of this dependency
#include <vector>
#include <stdint.h>

#ifndef MAX_ENTITIES
#define MAX_ENTITIES 65536
#endif

const uint32_t COMPONENT_LIST_SIZE_INCREMENT = 256;

struct EntityRef
{
    uint32_t list_idx;
    uint32_t entity_idx;
};

struct EntityHandle
{
    int32_t version = 0;  // 0 for uninitialized
    uint32_t idx;

    bool is_valid() const;
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

    EntityHandle pick_handle() const;

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
        uint32_t num_components_);

    ComponentInfo *component_info;
    uint32_t num_components;

    std::vector<EntityListInfo> entity_lists;
    EntityTable entity_table;

    EntityRef create_entity(
        uint32_t *required_components,
        uint32_t num_required_components);

    EntityRef create_entity_with_handle(
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

    uint32_t hec__default_new_list_action(uint32_t *required_components, uint32_t num_required_components);

    void kill_entity(
        EntityRef ref);
    void *lookup_component(
        EntityRef entity_ref,
        uint32_t cmp_type) const;
};

#endif  // HEC_HEADER_INCLUDED

#if defined(HEC_IMPLEMENTATION) && !defined(HEC_IMPLEMENTED)
#define HEC_IMPLEMENTED

#include <cstring>
#include <cassert>
#include <iostream>

// TODO: get rid of these
using std::cout;
using std::endl;

#define HEC_ALIGN 8

#define HEC_ARRAY_LENGTH(x) (sizeof((x)) / sizeof(*(x)))

bool EntityListInfo::supports_component(uint32_t component_type) const
{
    return components[component_type] != nullptr;
}

/////////////
// Handles //
/////////////

bool EntityHandle::is_valid() const
{
    return version != 0;
}

bool operator==(const EntityHandle &lhs, const EntityHandle &rhs)
{
    return lhs.version == rhs.version && lhs.idx == rhs.idx;
}

EntityHandle EntityTable::pick_handle() const
{
    // find a free entry
    uint32_t i;
    for (i = 0; i < HEC_ARRAY_LENGTH(used_entries); i++)
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


////////////////////
// Entity Manager //
////////////////////

EntityManager::EntityManager(
    ComponentInfo *components,
    uint32_t num_components_)
{
    component_info = components;
    num_components = num_components_;
}

#ifndef HEC_NEW_LIST_ACTION
#define HEC_NEW_LIST_ACTION(required_components, num_required_components) hec__default_new_list_action(required_components, num_required_components)
uint8_t hec__component_data[100 * 1024 * 1024];
size_t hec__component_data_size = sizeof(hec__component_data);
size_t hec__component_data_used = 0;

uint32_t EntityManager::hec__default_new_list_action(uint32_t *required_components, uint32_t num_required_components)
{
    EntityListInfo list_info;
    list_info.max_size = COMPONENT_LIST_SIZE_INCREMENT;

    size_t align_offset = (size_t)(hec__component_data + hec__component_data_used) % HEC_ALIGN;
    if (align_offset != 0)
    {
        hec__component_data_used += HEC_ALIGN - align_offset;
    }

    for (uint32_t i = 0; i < num_required_components; i++)
    {
        list_info.components[required_components[i]] = hec__component_data + hec__component_data_used;
        hec__component_data_used += COMPONENT_LIST_SIZE_INCREMENT * component_info[required_components[i]].size;
    }
    list_info.handles = (EntityHandle*)(hec__component_data + hec__component_data_used);
    hec__component_data_used += COMPONENT_LIST_SIZE_INCREMENT * sizeof(EntityHandle);

    assert(hec__component_data_used <= hec__component_data_size);

    uint32_t list_index = entity_lists.size();
    entity_lists.push_back(list_info);

    return list_index;
}
#endif // HEC_NEW_LIST_ACTION


EntityRef EntityManager::create_entity(
    uint32_t *required_components,
    uint32_t num_required_components)
{
    return create_entity_with_handle(
        required_components,
        num_required_components,
        entity_table.pick_handle());
}

EntityRef EntityManager::create_entity_with_handle(
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
        li = HEC_NEW_LIST_ACTION(required_components, num_required_components);
    }

    uint32_t entity_idx = entity_lists[li].size;

    entity_lists[li].handles[entity_idx] = handle;

    EntityRef ref;
    ref.entity_idx = entity_idx;
    ref.list_idx = li;

    entity_table.add_entry_with_handle(ref, handle);
    entity_lists[li].size++;

    return ref;
}

void EntityManager::kill_entity(EntityRef ref)
{
    // Lookup handle
    EntityHandle handle = entity_lists[ref.list_idx].handles[ref.entity_idx];

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

#endif
