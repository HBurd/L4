#ifndef HBPLANET_H
#define HBPLANET_H

#include "hb/entities.h"

Entity create_planet(Vec3 position, float radius, float mass);

#ifdef FAST_BUILD
#include "planet.cpp"
#endif

#endif
