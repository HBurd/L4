#include "hb/TransformFollowerComponent.h"

void update_transform_followers(
    EntityManager *entity_manager,
    EntityHandle *transform_followers,
    Transform *transforms,
    uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        // lookup transform to follow
        EntityRef target_ref = entity_manager->entity_table.lookup_entity(transform_followers[i]);
        Transform *target_transform = (Transform*)entity_manager->lookup_component(target_ref, ComponentType::TRANSFORM);
        transforms[i] = *target_transform;
    }
}
