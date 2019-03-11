#include "hb/player_control.h"
#include "hb/net.h"
#include "hb/packets.h"
#include "hb/util.h"
#include "imgui/imgui.h"
#include <cmath>

const float MAX_THRUST = 1.0f;
const float MAX_TORQUE = 1.0f;

PlayerControlState player_control_get_state(
    const Keyboard kb,
    bool stabilize,
    Transform player,
    bool track,
    Transform target)
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
        float roll = player.angular_velocity.x;   // xy
        float pitch = player.angular_velocity.y; // yz
        float yaw = player.angular_velocity.z;   // zx

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
 
    control_state.torque = Vec3(roll_torque, pitch_torque, yaw_torque);

    return control_state;
}

void player_control_update(Transform *transform, float mass, PlayerControlState control_state, float dt)
{
    Vec3 thrust = transform->orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    thrust = thrust * control_state.thrust;
    
    transform->velocity += thrust * (dt / mass);
    transform->angular_velocity += control_state.torque * (dt / mass);
}

void PlayerInputBuffer::save_input(PlayerControlState control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}
