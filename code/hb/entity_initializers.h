#ifndef HBENTITY_INITIALIZERS
#define HBENTITY_INITIALIZERS

void create_planet(Vec3 position, float radius, float mass, EntityManager *entity_manager);

EntityRef create_ship(Vec3 position, EntityManager *entity_manager);

EntityRef create_player_ship(Vec3 position, ClientId client_id, EntityManager *entity_manager);

EntityRef create_projectile(Transform shooter_transform, EntityManager *entity_manager);

#ifdef FAST_BUILD
#include "entity_initializers.cpp"
#endif

#endif
