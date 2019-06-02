#include "common/physics.h"

void apply_impulse(Vec3 impulse, Vec3 *velocity, float mass)
{
    *velocity += (1.0f / mass) * impulse;
}

void apply_angular_impulse(Vec3 angular_impulse, Vec3 *angular_velocity, float angular_mass)
{
    *angular_velocity += (1.0f / angular_mass) * angular_impulse;
}
