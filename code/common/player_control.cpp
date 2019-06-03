#include "client/player_input.h"
#include "common/net.h"
#include "common/packets.h"
#include "common/util.h"
#include "common/physics.h"
#include "common/components.h"

#include "client/keyboard.h"

#include "imgui/imgui.h"

#include <cmath>

void ShipControls::clamp()
{
    // clamp torque on each axis individually (assumed physical limitation on each axis)
    float factor = 1.0f;
    if (fabs(torque.x) > ShipControls::MAX_TORQUE)
    {
        factor = ShipControls::MAX_TORQUE / fabs(torque.x);
    }
    torque = factor * torque;

    factor = 1.0f;
    if (fabs(torque.y) > ShipControls::MAX_TORQUE)
    {
        factor = ShipControls::MAX_TORQUE / fabs(torque.y);
    }
    torque = factor * torque;

    factor = 1.0f;
    if (fabs(torque.z) > ShipControls::MAX_TORQUE)
    {
        factor = ShipControls::MAX_TORQUE / fabs(torque.z);
    }
    torque = factor * torque;

    // clamp thrust
    if (thrust > ShipControls::MAX_THRUST)
    {
        thrust = ShipControls::MAX_THRUST;
    }
    else if (thrust < 0.0f)
    {
        thrust = 0.0f;
    }
}


void get_ship_thrust(ShipControls controls, Rotor ship_orientation, Vec3 *thrust, Vec3 *torque)
{
    *thrust = ship_orientation.to_matrix() * Vec3(0.0f, 0.0f, -1.0f);
    *thrust = *thrust * controls.thrust;
    *torque = controls.torque;
}

void apply_ship_inputs(ShipControls inputs, Transform *transform, Physics physics, float dt)
{
    Vec3 thrust;
    Vec3 torque;
    get_ship_thrust(
        inputs,
        transform->orientation,
        &thrust,
        &torque);

    apply_impulse(
        thrust * dt,
        &transform->velocity,
        physics.mass);
    apply_angular_impulse(
        torque * dt,
        &transform->angular_velocity,
        physics.angular_mass);
}

void handle_player_input(EntityManager *entity_manager, EntityHandle player_handle, PlayerInputs player_inputs, float dt)
{
    EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
    assert(player_ref.is_valid());

    // Get the ship the player is controlling
    // For now this is determined by the transform it is following
    
    EntityHandle *player_ship_handle = (EntityHandle*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM_FOLLOWER);
    if (player_ship_handle)
    {
        // Player is piloting a ship
        EntityRef player_ship = entity_manager->entity_table.lookup_entity(*player_ship_handle);
        assert(player_ship.is_valid());

        Transform &ship_transform = *(Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
        Physics ship_physics = *(Physics*)entity_manager->lookup_component(player_ship, ComponentType::PHYSICS);

        if (player_inputs.leave_command_chair)
        {
            EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
            assert(player_ref.is_valid());
            entity_manager->remove_component(&player_ref, ComponentType::TRANSFORM_FOLLOWER);
        }

        apply_ship_inputs(player_inputs.ship, &ship_transform, ship_physics, dt);
    }
}
