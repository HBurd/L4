#pragma once

#include "common/math.h"

void apply_impulse(Vec3 impulse, Vec3 *velocity, float mass);

void apply_angular_impulse(Vec3 angular_impulse, Vec3 *angular_velocity, float angular_mass);

void apply_offset_impulse(Vec3 impulse, Vec3 offset, Vec3 *velocity, Vec3 *angular_velocity, float mass, float angular_mass);
