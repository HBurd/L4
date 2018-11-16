#include "hbplayer_control.h"

void player_control_update(Physics* physics, const Keyboard kb)
{
    float angle_speed = 0.01f;
    Rotor rotation;
    if (kb.q) rotation = Rotor::roll(-angle_speed) * rotation;
    if (kb.e) rotation = Rotor::roll(angle_speed) * rotation;
    if (kb.w) rotation = Rotor::pitch(angle_speed) * rotation;
    if (kb.s) rotation = Rotor::pitch(-angle_speed) * rotation;
    if (kb.a) rotation = Rotor::yaw(-angle_speed) * rotation;
    if (kb.d) rotation = Rotor::yaw(angle_speed) * rotation;

    physics->orientation = physics->orientation * rotation;

    float speed = 0.05f;
    Vec3 dp;
    if (kb.i) dp += speed * Vec3(0.0f, 0.0f, -1.0f);
    if (kb.k) dp += speed * Vec3(0.0f, 0.0f, 1.0f);

    dp = physics->orientation.to_matrix() * dp;
    physics->position += dp;
}
