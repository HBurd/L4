#include "hb/player_control.h"
#include "hb/net.h"
#include "hb/packets.h"
#include "hb/util.h"
#include "hb/physics.h"
#include "imgui/imgui.h"
#include <cmath>

const float MAX_THRUST = 1.0f;
const float MAX_TORQUE = 1.0f;

Vec3 compute_target_tracking_torque(
    Transform player_transform,
    Physics player_physics,
    Transform target_transform)
{
    /*  Tracking will point the ship in a desired orientation.
     *  It is achieved by finding the axis that rotates from the
     *  player's direction to the target direction, and applying
     *  torque on that axis using a PD controller (controlling angle).
     *  This does not work if the player's angular velocity has a
     *  component orthogonal to the plane of rotation, so a P controller
     *  is used additionally to neutralize the angular velocity in this
     *  direction.
     */
    Rotor player_orient_abs = player_transform.orientation;
    Vec3 player_dir_rel = Vec3(0.0f, 0.0f, -1.0f);
    Vec3 target_pos_abs = target_transform.position;
    Vec3 player_pos_abs = player_transform.position;
    Vec3 player_omega = player_transform.angular_velocity;

    Vec3 target_dir_rel =
        player_orient_abs.inverse().to_matrix()
        * (target_pos_abs - player_pos_abs).normalize();

    Rotor rotor(player_dir_rel, target_dir_rel);
    float angle;
    Vec3 axis;
    rotor.to_angle_axis(&angle, &axis);

    float derivative = dot(player_omega, axis);

    // these values were picked using pidTuner in matlab
    float torque = -0.96f * angle - 2.605f * derivative;


    Vec3 stabilize_axis = cross(axis, player_dir_rel);
    // didn't do any tuning here, just picked Kp = 1
    float stabilize_torque = -dot(player_omega, stabilize_axis);

    // angular mass essentially divides the OL gain so we must account for that
    torque *= player_physics.angular_mass;
    stabilize_torque *= player_physics.angular_mass;

    return torque * axis + stabilize_torque * stabilize_axis;
}

Vec3 compute_stabilization_torque(
    Transform transform,
    Physics physics)
{
    float roll  = transform.angular_velocity.x; // xy
    float pitch = transform.angular_velocity.y; // yz
    float yaw   = transform.angular_velocity.z; // zx

    return -Vec3(roll, pitch, yaw) * physics.angular_mass;
}

Vec3 compute_player_input_torque(
    Keyboard kb)
{
    float roll  = 0.0f;
    float pitch = 0.0f;
    float yaw   = 0.0f;
    if (kb.held.q) roll  = -MAX_TORQUE;
    if (kb.held.e) roll  = MAX_TORQUE;
    if (kb.held.w) pitch = MAX_TORQUE;
    if (kb.held.s) pitch = -MAX_TORQUE;
    if (kb.held.a) yaw   = -MAX_TORQUE;
    if (kb.held.d) yaw   = MAX_TORQUE;

    return Vec3(roll, pitch, yaw);
}

void PlayerControlState::clamp()
{
    // clamp torque on each axis individually (assumed physical limitation on each axis)
    float factor = 1.0f;
    if (fabs(torque.x) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(torque.x);
    }
    torque = factor * torque;

    factor = 1.0f;
    if (fabs(torque.y) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(torque.y);
    }
    torque = factor * torque;

    factor = 1.0f;
    if (fabs(torque.z) > MAX_TORQUE)
    {
        factor = MAX_TORQUE / fabs(torque.z);
    }
    torque = factor * torque;

    // clamp thrust
    if (thrust > MAX_THRUST)
    {
        thrust = MAX_THRUST;
    }
    else if (thrust < 0.0f)
    {
        thrust = 0.0f;
    }
}

float compute_player_input_thrust(Keyboard kb)
{
    float thrust;
    if (kb.held.space)
    {
        thrust = MAX_THRUST;
    }
    else
    {
        thrust = 0.0f;
    }

    return thrust;
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


void apply_ship_inputs(PlayerControlState inputs, Transform *transform, Physics physics)
{
    Vec3 thrust;
    Vec3 torque;
    get_ship_thrust(
        inputs,
        transform->orientation,
        &thrust,
        &torque);

    apply_impulse(
        thrust * (float)TIMESTEP,
        &transform->velocity,
        physics.mass);
    apply_angular_impulse(
        torque * (float)TIMESTEP,
        &transform->angular_velocity,
        physics.angular_mass);
}
