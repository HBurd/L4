#ifndef HBPHYSICS_H
#define HBPHYSICS_H

#include "hb/math.h"

void apply_impulse(Vec3 impulse, Vec3 *velocity, float mass);

void apply_angular_impulse(Vec3 angular_impulse, Vec3 *angular_velocity, float angular_mass);

#ifdef FAST_BUILD
#include "physics.cpp"
#endif

#endif // include guard
