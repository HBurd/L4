#ifndef HBPROJECTILE_H
#define HBPROJECTILE_H

#include "hb/entities.h"
#include "hb/renderer.h"
#include "hb/math.h"

Entity create_projectile(Transform shooter_transform);

// returns whether projectile still exists
bool projectile_update(Projectile* projectile);

#ifdef FAST_BUILD
#include "projectile.cpp"
#endif

#endif // include guard
