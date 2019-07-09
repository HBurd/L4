#include "client/player_input.h"
#include "common/util.h"
#include "common/components.h"

const float PLAYER_LOOK_FACTOR = 0.005f;

void PlayerInputBuffer::save_input(PlayerInputs input, float dt)
{
    inputs[next_seq_num % ARRAY_LENGTH(inputs)] = {input, dt, next_seq_num};
    next_seq_num++;
}

PlayerInputs process_player_inputs(const LocalGameData &game)
{
    EntityManager *entity_manager = game.entity_manager;
    EntityRef player_ship = lookup_player_ship(game.player_handle, entity_manager);

    PlayerInputs inputs;
        
    if (player_ship.is_valid())
    {
        inputs.type = PlayerInputs::InputType::SHIP;
        Transform ship_transform = *(Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
        Physics ship_physics = *(Physics*)entity_manager->lookup_component(player_ship, ComponentType::PHYSICS);

        if (game.tracking.track)
        {
            EntityRef target_ref = entity_manager->entity_table.lookup_entity(game.tracking.guidance_target);
            if (target_ref.is_valid())
            {
                Transform target_transform = *(Transform*)entity_manager->lookup_component(target_ref, ComponentType::TRANSFORM);

                inputs.ship.torque += compute_target_tracking_torque(
                    ship_transform,
                    ship_physics,
                    target_transform);
            }
        }
        else if (game.tracking.stabilize)
        {
            inputs.type = PlayerInputs::InputType::SHIP;
            inputs.ship.torque += compute_stabilization_torque(
                ship_transform,
                ship_physics);
        }

        // The player always has some control even when some controller is acting
        inputs.ship.torque += compute_player_input_torque(game.input.keyboard);
        inputs.ship.thrust += compute_player_input_thrust(game.input.keyboard);

        inputs.ship.clamp();

        inputs.ship.shoot = game.input.keyboard.down.enter;
    }
    else
    {
        Vec3 player_movement;
        if (game.input.keyboard.held.a) player_movement += Vec3(-1.0f, 0.0f, 0.0f);
        if (game.input.keyboard.held.d) player_movement += Vec3( 1.0f, 0.0f, 0.0f);
        if (game.input.keyboard.held.w) player_movement += Vec3( 0.0f, 0.0f,-1.0f);
        if (game.input.keyboard.held.s) player_movement += Vec3( 0.0f, 0.0f, 1.0f);
        if (game.input.keyboard.held.q) player_movement += Vec3( 0.0f,-1.0f, 0.0f); 
        if (game.input.keyboard.held.e) player_movement += Vec3( 0.0f, 1.0f, 0.0f); 

        // Set orientation in player input
        EntityRef player = entity_manager->entity_table.lookup_entity(game.player_handle);
        Transform *player_transform = (Transform*)entity_manager->lookup_component(player, ComponentType::TRANSFORM);
        inputs.player.orientation = player_transform->orientation * Rotor::yaw(PLAYER_LOOK_FACTOR * game.input.mouse.dx) * Rotor::pitch(PLAYER_LOOK_FACTOR * game.input.mouse.dy);;

        inputs.player.movement = player_movement;
        inputs.type = PlayerInputs::InputType::PLAYER;
    }

    return inputs;
}
