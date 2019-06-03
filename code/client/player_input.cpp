#include "client/player_input.h"

void PlayerInputBuffer::save_input(ShipControls control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}

ShipControls process_player_inputs(EntityManager *entity_manager, Keyboard kb, TrackingState tracking, EntityHandle ship_handle)
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
