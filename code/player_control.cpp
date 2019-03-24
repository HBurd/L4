#include "hb/player_control.h"
#include "hb/net.h"
#include "hb/packets.h"
#include "hb/util.h"
#include "hb/physics.h"
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
        /*  Tracking will point the ship in a desired orientation.
         *  It is achieved by finding the axis that rotates from the
         *  player's direction to the target direction, and applying
         *  torque on that axis using a PD controller (controlling angle).
         *  This does not work if the player's angular velocity has a
         *  component orthogonal to the plane of rotation, so a P controller
         *  is used additionally to neutralize the angular velocity in this
         *  direction.
         *
         *  Everything here is in the corrdinate frame of the player's ship.
         */
        Vec3 target_orientation = player.orientation.inverse().to_matrix() * (target.position - player.position).normalize();

        Vec3 player_orientation = Vec3(0.0f, 0.0f, -1.0f);
        
        Rotor rotor(player_orientation, target_orientation);
        float angle;
        Vec3 axis;
        rotor.to_angle_axis(&angle, &axis);

        float derivative = dot(player.angular_velocity, axis);

        // these values were picked using pidTuner in matlab
        float torque = -0.96 * angle - 2.605 * derivative;

        roll_torque  += torque * axis.x;
        pitch_torque += torque * axis.y;
        yaw_torque   += torque * axis.z;

        Vec3 stabilize_axis = cross(axis, player_orientation);
        // didn't do any tuning here, just picked Kp = 1
        float stabilize_torque = -dot(player.angular_velocity, stabilize_axis);
        roll_torque  += stabilize_torque * stabilize_axis.x;
        pitch_torque += stabilize_torque * stabilize_axis.y;
        yaw_torque   += stabilize_torque * stabilize_axis.z;
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

void get_ship_thrust(PlayerControlState input, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque)
{
    *thrust = ship_orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    *thrust = *thrust * input.thrust;
    *torque = input.torque;
}

void PlayerInputBuffer::save_input(PlayerControlState control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}
