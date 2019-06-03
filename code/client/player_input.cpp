#include "client/player_input.h"

void PlayerInputBuffer::save_input(ShipControls control_state, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {control_state, dt, next_seq_num};
    next_seq_num++;
}

PlayerInputs process_player_inputs(LocalGameData *game)
{
    EntityManager *entity_manager = game->entity_manager;
    EntityRef player_ship = entity_manager->entity_table.lookup_entity(game->player_ship_handle);

    assert(player_ship.is_valid());

    Transform ship_transform = *(Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
    Physics ship_physics = *(Physics*)entity_manager->lookup_component(player_ship, ComponentType::PHYSICS);

    PlayerInputs inputs;
    
    if (game->tracking.track)
    {
        EntityRef target_ref = entity_manager->entity_table.lookup_entity(game->tracking.guidance_target);
        if (target_ref.is_valid())
        {
            Transform target_transform = *(Transform*)entity_manager->lookup_component(target_ref, ComponentType::TRANSFORM);

            inputs.ship.torque += compute_target_tracking_torque(
                ship_transform,
                ship_physics,
                target_transform);
        }
    }
    else if (game->tracking.stabilize)
    {
        inputs.ship.torque += compute_stabilization_torque(
            ship_transform,
            ship_physics);
    }

    // The player always has some control even when some controller is acting
    inputs.ship.torque += compute_player_input_torque(game->input.keyboard);
    inputs.ship.thrust += compute_player_input_thrust(game->input.keyboard);

    inputs.ship.clamp();

    inputs.ship.shoot = game->input.keyboard.down.enter;

    return inputs;
}
