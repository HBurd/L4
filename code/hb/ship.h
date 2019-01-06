#ifndef HBSHIP_H
#define HBSHIP_H

#include "hb/entities.h"
#include "hb/renderer.h"
#include "hb/math.h"

Entity create_ship(Vec3 position);

#ifdef FAST_BUILD
#include "ship.cpp"
#endif

#endif // include guard
