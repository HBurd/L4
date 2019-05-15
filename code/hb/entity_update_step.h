#ifndef HBENTITY_UPDATE_STEP_H
#define HBENTITY_UPDATE_STEP_H

#include "hec.h"

void perform_entity_update_step(EntityManager *entity_manager, float dt);

#ifdef FAST_BUILD
#include "entity_update_step.cpp"
#endif

#endif
