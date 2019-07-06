#pragma once

#include "hec.h"

namespace ComponentType
{
    enum ComponentType
    {
        PHYSICS = 0,
        MESH,
        PLAYER_CONTROL,
        PROJECTILE,
        TRANSFORM,
        PLANET,
        WORLD_SECTOR,
        TRANSFORM_FOLLOWER,
        BOUNDING_BOX,

        // =======================================
        // Add components here as they are created
        // =======================================
        
        NUM_COMPONENT_TYPES
    };
}

void init_component_info(ComponentInfo *components, size_t num_components);

const char *component_name(uint32_t type);
void print_component(void *component, uint32_t type, char *buffer, size_t buffer_size);
