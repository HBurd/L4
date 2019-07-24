#include "common/TransformFollowerComponent.h"
#include "common/components.h"

void update_transform_followers(
    EntityManager *entity_manager,
    EntityHandle *transform_followers,
    Transform *transforms,
    WorldSector *position_rfs,
    uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        // lookup transform to follow
        EntityRef target_ref = entity_manager->entity_table.lookup_entity(transform_followers[i]);

        Transform *target_transform = (Transform*)entity_manager->lookup_component(target_ref, ComponentType::TRANSFORM);
        transforms[i] = *target_transform;

        WorldSector *target_position_rf = (WorldSector*)entity_manager->lookup_component(target_ref, ComponentType::WORLD_SECTOR);
        position_rfs[i] = *target_position_rf;
    }
}
