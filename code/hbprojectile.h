#ifndef HBPROJECTILE_H
#define HBPROJECTILE_H

#include "hbentities.h"
#include "hbrenderer.h"
#include "hbmath.h"

Entity create_projectile(Physics shooter_physics, MeshId mesh_id);

// returns whether projectile still exists
bool projectile_update(Projectile* projectile);

#endif // include guard
