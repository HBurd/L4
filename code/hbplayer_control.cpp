#include "hbplayer_control.h"
#include "hbnet.h"
#include "imgui/imgui.h"
#include <cmath>

const float MAX_THRUST = 0.001f;
const float MAX_TORQUE = 0.001f;

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Physics player,
    bool track,
    Physics target)
{
    // build the control packet and send it to the server

    PlayerControlState control_state;
    if (kb.held.space)
    {
        control_state.thrust = MAX_THRUST;
    }

    float roll_torque = 0.0f;
    float pitch_torque = 0.0f;
    float yaw_torque = 0.0f;

    if (track)
    {
        Vec3 target_direction = target.velocity - player.velocity;
        if (target_direction.norm() >= 0.0001f)
        {
            // find the rotor representing the rotation to the thrust vector
            Rotor err = Rotor(player.orientation.to_matrix() * Vec3(0.0f, 0.0f, 1.0f),
                              target_direction.normalize());
            

        }
    }
    else if (stabilize)  // the two do not work together
    {
        float roll = player.angular_velocity.xy;
        float pitch = player.angular_velocity.yz;
        float yaw = player.angular_velocity.zx;

        roll_torque = -roll;
        pitch_torque = -pitch;
        yaw_torque = -yaw;
    }
 
    if (kb.held.q) roll_torque = -MAX_TORQUE;
    if (kb.held.e) roll_torque = MAX_TORQUE;
    if (kb.held.w) pitch_torque = MAX_TORQUE;
    if (kb.held.s) pitch_torque = -MAX_TORQUE;
    if (kb.held.a) yaw_torque = -MAX_TORQUE;
    if (kb.held.d) yaw_torque = MAX_TORQUE;

    // clamp torque

    float factor = 1.0f;
    if (fabs(roll_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(roll_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;

    factor = 1.0f;
    if (fabs(pitch_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(pitch_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;

    factor = 1.0f;
    if (fabs(yaw_torque) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(yaw_torque);
    }
    roll_torque *= factor;
    pitch_torque *= factor;
    yaw_torque *= factor;
 
    control_state.torque = Rotor::roll(roll_torque) * Rotor::pitch(pitch_torque) * Rotor::yaw(yaw_torque);

    return control_state;
}

void player_control_update(Physics* physics, PlayerControlState control_state)
{
    Vec3 thrust = physics->orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    thrust = thrust * control_state.thrust;
    
    physics->velocity += thrust;
    physics->angular_velocity = physics->angular_velocity * control_state.torque;
}
