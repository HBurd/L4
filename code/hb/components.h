#ifndef HBCOMPONENTS_H
#define HBCOMPONENTS_H

#include "hb/TransformComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/PlanetComponent.h"
#include "hb/ProjectileComponent.h"

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
        VIEW_ORIENTATION,

        // =======================================
        // Add components here as they are created
        // =======================================
        
        NUM_COMPONENT_TYPES
    };
}

struct ComponentInfo
{
    uint32_t size;
};

void init_component_info(ComponentInfo *components, size_t num_components);

#endif
