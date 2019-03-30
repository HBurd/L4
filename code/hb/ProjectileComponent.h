#ifndef HBPROJECTILE_COMPONENT_H
#define HBPROJECTILE_COMPONENT_H

#define PROJECTILE_COMPONENT Projectile, projectile, PROJECTILE
struct Projectile
{
    unsigned int timeout = 60; // 1 second
};

//#ifdef FAST_BUILD
//#include "ProjectileComponent.h"
//#endif

#endif
