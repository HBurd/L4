#ifndef HBSHIP_H
#define HBSHIP_H

#include "hec.h"
#include "hb/renderer.h"
#include "hb/math.h"
#include "hb/net.h"

EntityRef create_ship(Vec3 position, EntityManager *entity_manager);
EntityRef create_player_ship(Vec3 position, ClientId client_id, EntityManager *entity_manager);

#ifdef FAST_BUILD
#include "ship.cpp"
#endif

#endif // include guard
