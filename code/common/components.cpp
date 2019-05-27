#include "hb/components.h"


void init_component_info(ComponentInfo *components, size_t num_components)
{
    for (uint32_t i = 0; i < num_components; i++)
    {
        switch(i)
        {
            case ComponentType::WORLD_SECTOR:
                components[i] = {sizeof(WorldSector)};
                break;
            case ComponentType::TRANSFORM:
                components[i] = {sizeof(Transform)};
                break;
            case ComponentType::PLAYER_CONTROL:
                components[i] = {sizeof(PlayerControl)};
                break;
            case ComponentType::PLANET:
                components[i] = {sizeof(Planet)};
                break;
            case ComponentType::PHYSICS:
                components[i] = {sizeof(Physics)};
                break;
            case ComponentType::PROJECTILE:
                components[i] = {sizeof(Projectile)};
                break;
            case ComponentType::MESH:
                components[i] = {sizeof(MeshId)};
                break;
            case ComponentType::TRANSFORM_FOLLOWER:
                components[i] = {sizeof(EntityHandle)};
                break;
            default:
                assert(false);
        }
    }
}
