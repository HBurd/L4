#ifndef HBPROJECTILE_H
#define HBPROJECTILE_H

#include "hec.h"
#include "hb/renderer.h"
#include "hb/math.h"

EntityRef create_projectile(Transform shooter_transform, EntityManager *entity_manager);

// returns whether projectile still exists
bool projectile_update(Projectile* projectile);

#ifdef FAST_BUILD
#include "projectile.cpp"
#endif

#endif // include guard
