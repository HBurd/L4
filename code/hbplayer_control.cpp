#include "hbplayer_control.h"
#include "hbnet.h"

PlayerControlState player_control_get_state(const Keyboard kb)
{
    // build the control packet and send it to the server
    float max_thrusdt = 0.05f;
    float angle_speed = 0.01f;

    PlayerControlState control_state;
    if (kb.space)
    {
        control_state.thrust = 0.05f;
    }
    if (kb.q) control_state.torque = Rotor::roll(-angle_speed)  * control_state.torque;
    if (kb.e) control_state.torque = Rotor::roll(angle_speed)   * control_state.torque;
    if (kb.w) control_state.torque = Rotor::pitch(angle_speed)  * control_state.torque;
    if (kb.s) control_state.torque = Rotor::pitch(-angle_speed) * control_state.torque;
    if (kb.a) control_state.torque = Rotor::yaw(-angle_speed)   * control_state.torque;
    if (kb.d) control_state.torque = Rotor::yaw(angle_speed)    * control_state.torque;

    return control_state;

    //float angle_speed = 0.01f;
    //Rotor rotation;
    //if (kb.q) rotation = Rotor::roll(-angle_speed) * rotation;
    //if (kb.e) rotation = Rotor::roll(angle_speed) * rotation;
    //if (kb.w) rotation = Rotor::pitch(angle_speed) * rotation;
    //if (kb.s) rotation = Rotor::pitch(-angle_speed) * rotation;
    //if (kb.a) rotation = Rotor::yaw(-angle_speed) * rotation;
    //if (kb.d) rotation = Rotor::yaw(angle_speed) * rotation;

    //physics->orientation = physics->orientation * rotation;

    //float speed = 0.05f;
    //Vec3 dp;
    //if (kb.i) dp += speed * Vec3(0.0f, 0.0f, -1.0f);
    //if (kb.k) dp += speed * Vec3(0.0f, 0.0f, 1.0f);

    //dp = physics->orientation.to_matrix() * dp;
    //physics->position += dp;
}

void player_control_update(Physics* physics, PlayerControlState control_state)
{
    Vec3 thrust = physics->orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    thrust = thrust * control_state.thrust;
    
    physics->position += thrust;
    physics->orientation= physics->orientation * control_state.torque;
}
