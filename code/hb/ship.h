#ifndef HBSHIP_H
#define HBSHIP_H

#include "hb/entities.h"
#include "hb/renderer.h"
#include "hb/math.h"

EntityHandle create_ship(Vec3 position, EntityManager *entity_manager);
EntityHandle create_player_ship(Vec3 position, ClientId client_id, EntityManager *entity_manager);

#ifdef FAST_BUILD
#include "ship.cpp"
#endif

#endif // include guard
