#ifndef HBPLANET_H
#define HBPLANET_H

#include "hec.h"

void create_planet(Vec3 position, float radius, float mass, EntityManager *entity_manager);

#ifdef FAST_BUILD
#include "planet.cpp"
#endif

#endif
