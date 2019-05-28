#ifndef HBPROJECTILE_H
#define HBPROJECTILE_H

#include "hec.h"
#include "hb/renderer.h"
#include "hb/math.h"
#include "hb/TransformComponent.h"
#include "hb/ProjectileComponent.h"

EntityRef create_projectile(Transform shooter_transform, EntityManager *entity_manager);

// returns whether projectile still exists
bool projectile_update(Projectile *projectile);

#endif // include guard
