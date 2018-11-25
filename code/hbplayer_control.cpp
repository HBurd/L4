#include "hbplayer_control.h"
#include "hbnet.h"

PlayerControlState player_control_get_state(const Keyboard kb)
{
    // build the control packet and send it to the server
    float max_thrust = 0.001f;
    float angle_speed = 0.001f;

    PlayerControlState control_state;
    if (kb.space)
    {
        control_state.thrust = max_thrust;
    }
    if (kb.q) control_state.torque = Rotor::roll(-angle_speed)  * control_state.torque;
    if (kb.e) control_state.torque = Rotor::roll(angle_speed)   * control_state.torque;
    if (kb.w) control_state.torque = Rotor::pitch(angle_speed)  * control_state.torque;
    if (kb.s) control_state.torque = Rotor::pitch(-angle_speed) * control_state.torque;
    if (kb.a) control_state.torque = Rotor::yaw(-angle_speed)   * control_state.torque;
    if (kb.d) control_state.torque = Rotor::yaw(angle_speed)    * control_state.torque;

    return control_state;
}

void player_control_update(Physics* physics, PlayerControlState control_state)
{
    Vec3 thrust = physics->orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    thrust = thrust * control_state.thrust;
    
    physics->velocity += thrust;
    physics->angular_velocity = physics->angular_velocity * control_state.torque;
}
