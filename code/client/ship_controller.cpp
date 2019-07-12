#include "client/ship_controller.h"
#include "common/player_control.h"

Vec3 compute_target_tracking_torque(
    Transform player_transform,
    WorldSector player_reference_frame,
    Physics player_physics,
    Transform target_transform,
    WorldSector target_reference_frame)
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
    Vec3 target_pos_abs = relative_to_sector(player_reference_frame, target_reference_frame, target_transform.position);
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
    if (kb.held.q) roll  = -ShipControls::MAX_TORQUE;
    if (kb.held.e) roll  =  ShipControls::MAX_TORQUE;
    if (kb.held.w) pitch =  ShipControls::MAX_TORQUE;
    if (kb.held.s) pitch = -ShipControls::MAX_TORQUE;
    if (kb.held.a) yaw   = -ShipControls::MAX_TORQUE;
    if (kb.held.d) yaw   =  ShipControls::MAX_TORQUE;

    return Vec3(roll, pitch, yaw);
}

float compute_player_input_thrust(Keyboard kb)
{
    float thrust;
    if (kb.held.space)
    {
        thrust = ShipControls::MAX_THRUST;
    }
    else
    {
        thrust = 0.0f;
    }

    return thrust;
}
