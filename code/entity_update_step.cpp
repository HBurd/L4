#include "hb/entity_update_step.h"
#include "hb/projectile.h"

void perform_entity_update_step(EntityManager *entity_manager, float dt)
{
    // Transform updates
    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        EntityList& entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_components(ComponentType::TRANSFORM))
            continue;
        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            Transform &transform = entity_list.transform_list[entity_idx];
            transform.position += transform.velocity * dt;

            Rotor delta_rotor = Rotor::angle_axis(dt * transform.angular_velocity);
            transform.orientation = transform.orientation * delta_rotor;
            
            // we need to normalize orientation and angular velocity every frame,
            // or we get accumulating errors
            transform.orientation = transform.orientation.normalize();
        }
    }

    // Projectile updates
    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        EntityList& entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_components(ComponentType::PROJECTILE))
            continue;
        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            if (!projectile_update(&entity_list.projectile_list[entity_idx]))
            {
                // TODO: synchronize between clients and server?
                // also will this break the loops?
                entity_manager->kill_entity(entity_list.handles[entity_idx]);
            }
        }
    }
}
