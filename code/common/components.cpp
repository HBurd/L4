#include "common/components.h"
#include "common/TransformComponent.h"
#include "common/PlanetComponent.h"
#include "common/PhysicsComponent.h"
#include "common/PlayerControlComponent.h"
#include "common/ProjectileComponent.h"
#include "common/renderer.h"
#include "common/collision.h"

#include <cassert>
#include <cinttypes>

void init_component_info(ComponentInfo *components, size_t num_components)
{
    for (uint32_t i = 0; i < num_components; i++)
    {
        switch(i)
        {
            case ComponentType::WORLD_SECTOR:
                components[i].size = sizeof(WorldSector);
                break;
            case ComponentType::TRANSFORM:
                components[i].size = sizeof(Transform);
                break;
            case ComponentType::PLAYER_CONTROL:
                components[i].size = sizeof(PlayerControl);
                break;
            case ComponentType::PLANET:
                components[i].size = sizeof(Planet);
                break;
            case ComponentType::PHYSICS:
                components[i].size = sizeof(Physics);
                break;
            case ComponentType::PROJECTILE:
                components[i].size = sizeof(Projectile);
                break;
            case ComponentType::MESH:
                components[i].size = sizeof(MeshId);
                break;
            case ComponentType::TRANSFORM_FOLLOWER:
                components[i].size = sizeof(EntityHandle);
                break;
            case ComponentType::BOUNDING_BOX:
                components[i].size = sizeof(BoundingBox);
                break;
            default:
                assert(false);
        }
    }
}

const char *component_name(uint32_t type)
{
    const char *names[ComponentType::NUM_COMPONENT_TYPES] = {};
    names[ComponentType::WORLD_SECTOR]       = "WorldSector";
    names[ComponentType::TRANSFORM]          = "Transform";
    names[ComponentType::PLAYER_CONTROL]     = "PlayerControl";
    names[ComponentType::PLANET]             = "Planet";
    names[ComponentType::PHYSICS]            = "Physics";
    names[ComponentType::PROJECTILE]         = "Projectile";
    names[ComponentType::MESH]               = "MeshId";
    names[ComponentType::TRANSFORM_FOLLOWER] = "TransformFollower";
    names[ComponentType::BOUNDING_BOX]       = "BoundingBox";
    return names[type];
}

void print_component(void *component, uint32_t type, char *buffer, size_t buffer_size)
{
    switch (type)
    {
        case ComponentType::WORLD_SECTOR:
        {
            WorldSector *sector = (WorldSector*)component;
            snprintf(buffer, buffer_size, "Sector %" PRId64 ", %" PRId64 ", %" PRId64, sector->x, sector->y, sector->z);
        } break;
        case ComponentType::TRANSFORM:
        {
            Transform *transform = (Transform*)component;
            snprintf(buffer, buffer_size,
                "Position:    (%f, %f, %f)\n"
                "Velocity:    (%f, %f, %f)\n"
                "Orientation: (%f, %f, %f, %f)\n"
                "Scale:       (%f, %f, %f)\n",
                transform->position.x, transform->position.y, transform->position.z,
                transform->velocity.x, transform->velocity.y, transform->velocity.z,
                transform->orientation.s, transform->orientation.xy, transform->orientation.yz, transform->orientation.zx,
                transform->scale.x, transform->scale.y, transform->scale.z);

        } break;
        default:
            snprintf(buffer, buffer_size, "This component has no defined print method.");
    }
}
