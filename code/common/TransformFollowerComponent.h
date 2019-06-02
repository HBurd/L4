#ifndef HBTRANSFORM_FOLLOWER_COMPONENT_H
#define HBTRANSFORM_FOLLOWER_COMPONENT_H

#include "hec.h"
#include "TransformComponent.h"

void update_transform_followers(
    EntityManager *entity_manager,
    EntityHandle *transform_followers,
    Transform *transforms,
    uint32_t count);

#endif // include guard
