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
    if (player_inputs.leave_command_chair)
    {
        EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
        assert(player_ref.is_valid());
        entity_manager->remove_component(&player_ref, ComponentType::TRANSFORM_FOLLOWER);
    }

    apply_input(player_inputs, player_handle, dt, entity_manager);
}

void apply_input(PlayerInputs input, EntityHandle handle, float dt, EntityManager *entity_manager)
{
    EntityRef entity = entity_manager->entity_table.lookup_entity(handle);
    switch (input.type)
    {
        case PlayerInputs::InputType::PLAYER:
        {
            Transform *transform = (Transform*)entity_manager->lookup_component(entity, ComponentType::TRANSFORM);
            transform->velocity = transform->orientation.to_matrix() * input.player.movement;
            transform->orientation = input.player.orientation;
        } break;
        case PlayerInputs::InputType::SHIP:
        {
            // TODO: ship input should probably be associated with the ship, not player
            EntityHandle *ship_handle = (EntityHandle*)entity_manager->lookup_component(entity, ComponentType::TRANSFORM_FOLLOWER);
            if (ship_handle)
            {
                EntityRef ship = entity_manager->entity_table.lookup_entity(*ship_handle);
                assert(ship.is_valid());

                Transform *transform = (Transform*)entity_manager->lookup_component(ship, ComponentType::TRANSFORM);
                Physics *physics = (Physics*)entity_manager->lookup_component(ship, ComponentType::PHYSICS);
                
                apply_ship_inputs(input.ship, transform, *physics, dt);
            }
        } break;
    }
}

EntityRef lookup_player_ship(EntityHandle player_handle, EntityManager *entity_manager)
{
    EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
    if (!player_ref.is_valid())
    {
        return EntityRef();
    }
 
    EntityHandle *player_ship_handle = (EntityHandle*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM_FOLLOWER);

    // Verify the player is following a transform (hopefully the ship)
    if (!player_ship_handle)
    {
        return EntityRef();
    }

    return entity_manager->entity_table.lookup_entity(*player_ship_handle);
}
