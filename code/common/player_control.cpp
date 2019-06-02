#include "common/player_control.h"
#include "common/net.h"
#include "common/packets.h"
#include "common/util.h"
#include "common/physics.h"
#include "imgui/imgui.h"
#include "client/keyboard.h"
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

void ShipControls::clamp()
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

void get_ship_thrust(ShipControls controls, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque)
{
    *thrust = ship_orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    *thrust = *thrust * controls.thrust;
    *torque = controls.torque;
}

void PlayerInputBuffer::save_input(ShipControls control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}


void apply_ship_inputs(ShipControls inputs, Transform *transform, Physics physics)
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

ShipControls ship_control(EntityManager *entity_manager, Keyboard kb, TrackingState tracking, EntityHandle ship_handle)
{
    EntityRef player_ship = entity_manager->entity_table.lookup_entity(ship_handle);

    assert(player_ship.is_valid());

    Transform ship_transform = *(Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
    Physics ship_physics = *(Physics*)entity_manager->lookup_component(player_ship, ComponentType::PHYSICS);

    ShipControls control_state;
    
    if (tracking.track)
    {
        EntityRef target_ref = entity_manager->entity_table.lookup_entity(tracking.guidance_target);
        if (target_ref.is_valid())
        {
            Transform target_transform = *(Transform*)entity_manager->lookup_component(target_ref, ComponentType::TRANSFORM);

            control_state.torque += compute_target_tracking_torque(
                ship_transform,
                ship_physics,
                target_transform);
        }
    }
    else if (tracking.stabilize)
    {
        control_state.torque += compute_stabilization_torque(
            ship_transform,
            ship_physics);
    }

    // The player always has some control even when some controller is acting
    control_state.torque += compute_player_input_torque(kb);
    control_state.thrust += compute_player_input_thrust(kb);

    control_state.clamp();

    control_state.shoot = kb.down.enter;

    return control_state;
}

void handle_player_input(EntityManager *entity_manager, EntityHandle player_handle, PlayerInputs player_inputs)
{
    EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
    assert(player_ref.is_valid());

    // Get the ship the player is controlling
    // For now this is determined by the transform it is following
    
    EntityHandle *player_ship_handle = (EntityHandle*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM_FOLLOWER);
    if (player_ship_handle)
    {
        EntityRef player_ship = entity_manager->entity_table.lookup_entity(*player_ship_handle);
        assert(player_ship.is_valid());

        Transform &ship_transform = *(Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
        Physics ship_physics = *(Physics*)entity_manager->lookup_component(player_ship, ComponentType::PHYSICS);

        apply_ship_inputs(player_inputs.ship, &ship_transform, ship_physics);
    }

    if (player_inputs.leave_command_chair)
    {

        EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
        assert(player_ref.is_valid());
        entity_manager->remove_component(&player_ref, ComponentType::TRANSFORM_FOLLOWER);
    }
}
