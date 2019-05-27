#ifndef HBCOMPONENTS_H
#define HBCOMPONENTS_H

#include "hb/TransformComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/PlanetComponent.h"
#include "hb/ProjectileComponent.h"

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

        // =======================================
        // Add components here as they are created
        // =======================================
        
        NUM_COMPONENT_TYPES
    };
}

void init_component_info(ComponentInfo *components, size_t num_components);

#endif
