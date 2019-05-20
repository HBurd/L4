#ifndef HBPLANET_COMPONENT_H
#define HBPLANET_COMPONENT_H

struct Planet
{
    float radius = 1.0f;
    float mass = 1.0f;

    Planet() = default;
    Planet(float planet_radius, float planet_mass);
};

#endif
