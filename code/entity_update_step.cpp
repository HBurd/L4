#include "hb/entity_update_step.h"
#include "hb/projectile.h"
#include "hb/TransformComponent.h"

void perform_entity_update_step(EntityManager *entity_manager, float dt)
{
    // Transform updates
    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
        if(!entity_list.supports_component(ComponentType::WORLD_SECTOR)) continue;
        if(!entity_list.supports_component(ComponentType::TRANSFORM)) continue;

        update_transform_components(
            (WorldSector*)entity_list.components[ComponentType::WORLD_SECTOR],
            (Transform*)entity_list.components[ComponentType::TRANSFORM],
            entity_list.size,
            dt);
    }

    // Projectile updates
    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_component(ComponentType::PROJECTILE)) continue;

        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            if (!projectile_update((Projectile*)(entity_list.components[ComponentType::PROJECTILE]) + entity_idx))
            {
                // TODO: synchronize between clients and server?
                // also will this break the loops?
                entity_manager->kill_entity(entity_list.handles[entity_idx]);
            }
        }
    }
}
