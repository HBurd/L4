#ifndef HBCOMPONENTS_H
#define HBCOMPONENTS_H

#include "hb/TransformComponent.h"
#include "hb/PhysicsComponent.h"
#include "hb/PlayerControlComponent.h"
#include "hb/MeshComponent.h"
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

        // =======================================
        // Add components here as they are created
        // =======================================
        
        NUM_COMPONENT_TYPES
    };
}

#endif
