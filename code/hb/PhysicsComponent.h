#ifndef HBPHYSICS_COMPONENT_H
#define HBPHYSICS_COMPONENT_H

#define PHYSICS_COMPONENT Physics, physics, PHYSICS
struct Physics
{
    float mass = 1.0f;
    float angular_mass = 1.0f; // moment of ineria
};

//#ifdef FAST_BUILD
//#include "PhysicsComponent.cpp"
//#endif

#endif
