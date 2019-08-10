#pragma once

#include "hec.h"
#include "common/math.h"
#include "common/TransformComponent.h"
#include "common/ProjectileComponent.h"

EntityRef create_projectile(Transform shooter_transform, EntityManager *entity_manager);

// returns whether projectile still exists
bool projectile_update(Projectile *projectile);
