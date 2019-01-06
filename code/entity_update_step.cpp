#include "hb/entity_update_step.h"
#include "hb/projectile.h"

void perform_entity_update_step(EntityManager *entity_manager, double dt)
{
    // Physics updates
    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        EntityList& entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_components(ComponentType::PHYSICS))
            continue;
        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            Physics& physics = entity_list.physics_list[entity_idx];
            physics.position += physics.velocity * dt;
            physics.orientation = physics.orientation
                * Rotor::lerp(Rotor(), physics.angular_velocity, dt);
            
            // we need to normalize orientation and angular velocity every frame,
            // or we get accumulating floating point errors
            physics.orientation = physics.orientation.normalize();
            physics.angular_velocity = physics.angular_velocity.normalize();
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
