#include "common/physics.h"

void apply_impulse(Vec3 impulse, Vec3 *velocity, float mass)
{
    *velocity += (1.0f / mass) * impulse;
}

void apply_angular_impulse(Vec3 angular_impulse, Vec3 *angular_velocity, float angular_mass)
{
    *angular_velocity += (1.0f / angular_mass) * angular_impulse;
}

void apply_offset_impulse(Vec3 impulse, Vec3 offset, Vec3 *velocity, Vec3 *angular_velocity, float mass, float angular_mass)
{
    apply_impulse(impulse, velocity, mass);

    // TODO: There is absolutely a bug with how angular velocity is implemented, making this necessary:
    Vec3 angular_impulse = cross(impulse, offset);
    float temp = angular_impulse.x;
    angular_impulse.x = angular_impulse.z;
    angular_impulse.z = angular_impulse.y;
    angular_impulse.y = temp;
    apply_angular_impulse(angular_impulse, angular_velocity, angular_mass);
}
