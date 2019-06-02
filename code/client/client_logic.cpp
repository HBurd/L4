#include "client/client_logic.h"
#include "common/PlayerControlComponent.h"
#include "common/components.h"
#include "common/util.h"
#include "common/physics.h"
#include "common/packets.h"

LocalGameData::LocalGameData(EntityManager *entity_manager_)
    : entity_manager(entity_manager_) {}

void handle_entity_create(LocalGameData *game, ClientData *client, GamePacketIn packet)
{
    EntityManager *entity_manager = game->entity_manager;

    entity_manager->create_entity_from_serialized(
        packet.packet.packet_data.entity_create.entity_data,
        packet.packet.packet_data.entity_create.data_size,
        packet.packet.packet_data.entity_create.handle);

    // Check if created entity is the player entity
    EntityRef ref = entity_manager->entity_table.lookup_entity(
        packet.packet.packet_data.entity_create.handle);
    if (ref.is_valid() && entity_manager->entity_lists[ref.list_idx].supports_component(ComponentType::PLAYER_CONTROL)
        && ((PlayerControl*)entity_manager->lookup_component(ref, ComponentType::PLAYER_CONTROL))->client_id == client->id)
    {
        assert(!game->player_handle.is_valid());
        assert(!game->player_ship_handle.is_valid());

        game->player_handle = packet.packet.packet_data.entity_create.handle;
        game->player_ship_handle =
            *(EntityHandle*)entity_manager->lookup_component(
                    entity_manager->entity_table.lookup_entity(
                        game->player_handle),
                    ComponentType::TRANSFORM_FOLLOWER);
    }
}

void handle_physics_sync(LocalGameData *game, PlayerInputBuffer *past_inputs, GamePacketIn packet)
{
    EntityManager *entity_manager = game->entity_manager;

    EntityHandle sync_entity = packet.packet.packet_data.transform_sync.entity;
    EntityRef sync_ref = entity_manager->entity_table.lookup_entity(sync_entity);
    if (!sync_ref.is_valid())
    {
        return;
    }

    Transform &transform = *(Transform*)entity_manager->lookup_component(sync_ref, ComponentType::TRANSFORM);
    // if the update is for the player, then selectively apply it
    if (sync_entity == game->player_ship_handle)
    {
        // if we have received no later transform syncs from the server
        if (packet.packet.packet_data.transform_sync.sequence
            > past_inputs->last_received_seq_num)
        {
            // Apply the transform update
            transform = 
                packet.packet.packet_data.transform_sync.transform_state;
            past_inputs->last_received_seq_num =
                packet.packet.packet_data.transform_sync.sequence;

            // apply later inputs (reconciliation)
            for (uint32_t sequence = packet.packet.packet_data.transform_sync.sequence;
                 sequence < past_inputs->next_seq_num;
                 sequence++)
            {
                uint32_t input_idx = sequence % ARRAY_LENGTH(past_inputs->inputs);
                if (past_inputs->inputs[input_idx].sequence_number == sequence)
                {
                    Physics physics = *(Physics*)entity_manager->lookup_component(sync_ref, ComponentType::PHYSICS);

                    apply_ship_inputs(past_inputs->inputs[input_idx].input, &transform, physics);
                }
            }
        }
    }
    else    // if the update isn't for the player then always apply it
    {
        transform = packet.packet.packet_data.transform_sync.transform_state;
    }
}
