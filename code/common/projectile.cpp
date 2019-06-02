#include "common/projectile.h"

bool projectile_update(Projectile* projectile)
{
    projectile->timeout--;
    return projectile->timeout ? true : false;
}
