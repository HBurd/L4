#include "SDL/SDL.h"
#include <iostream>
#include <cassert>

#include "hec.h"

#include "common/time.h"
#include "common/util.h"
#include "common/packets.h"
#include "common/entity_update_step.h"
#include "common/entity_initializers.h"
#include "common/components.h"
#include "common/TransformFollowerComponent.h"

#include "client/player_input.h"

#include "server/server.h"

#undef main

const double TIMESTEP = 1.0 / 60.0;

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

void handle_player_spawn_req(EntityManager *entity_manager, ServerData *server, GamePacket *packet)
{
    EntityRef ship_ref = create_ship(packet->packet_data.player_spawn.coords, entity_manager);
    EntityHandle ship_handle = entity_manager->entity_lists[ship_ref.list_idx].handles[ship_ref.entity_idx];

    // create player entity
    uint32_t player_components[] = {
        ComponentType::WORLD_SECTOR,
        ComponentType::TRANSFORM,
        ComponentType::TRANSFORM_FOLLOWER
    };
    
    EntityRef player_ref = entity_manager->create_entity(player_components, ARRAY_LENGTH(player_components));
    Transform *player_transform = (Transform*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM);
    *player_transform = Transform();
    WorldSector *player_sector = (WorldSector*)entity_manager->lookup_component(player_ref, ComponentType::WORLD_SECTOR);
    *player_sector = WorldSector();
    EntityHandle *transform_follower = (EntityHandle*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM_FOLLOWER);
    *transform_follower = ship_handle;

    EntityHandle player_handle = entity_manager->entity_lists[player_ref.list_idx].handles[player_ref.entity_idx];

    server->clients[packet->header.sender].player_entity = player_handle;

    uint8_t *create_packet_data = new uint8_t[2048];
    size_t create_packet_size = make_entity_create_packet(ship_ref, entity_manager, INCOMPLETE_ID, create_packet_data, 2048);
    
    server->broadcast(
        GamePacketType::ENTITY_CREATE,
        create_packet_data,
        create_packet_size);

    create_packet_size = make_entity_create_packet(player_ref, entity_manager, packet->header.sender, create_packet_data, 2048);

    server->broadcast(
        GamePacketType::ENTITY_CREATE,
        create_packet_data,
        create_packet_size);

    delete[] create_packet_data;
}

int main(int argc, char* argv[])
{
    // parse command line arguments
    uint16_t port;
    {
        if (argc != 2)
        {
            cout << "Usage: L4Server [port]" << endl;
            return 1;
        }
        uint32_t port32 = atoi(argv[1]);
        if (port32 == 0 || port32 & 0xFFFF0000)
        {
            cout << "A valid port was not supplied." << endl;
            return 1;
        }

        port = (uint16_t) port32;
    }

    ServerData server(port);

    vector<GamePacketIn> game_packets;

    ComponentInfo components[ComponentType::NUM_COMPONENT_TYPES];
    init_component_info(components, ARRAY_LENGTH(components));

    EntityManager *entity_manager =
        new EntityManager(
            components,
            ARRAY_LENGTH(components));

    create_planet(Vec3(0.0f, 0.0f, -1005.0f), 1000.0f, 10000.0f, entity_manager);
 
    TimeKeeper time_keeper;

    bool running = true;
    while (running)
    {
        // Process incoming packets
        get_packets(server.sock, &game_packets);
        for (auto packet : game_packets)
        {
            switch (packet.packet.header.type)
            {
                case GamePacketType::CONNECTION_REQ:
                {
                    ClientId client_id = server.accept_client(packet.sender);
                    if (client_id == INCOMPLETE_ID)
                    {
                        break;
                    }
                    cout << "Client connected with id " << client_id << endl;
                    // now update the client with all existing entities
                    for (size_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
                    {
                        EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
                        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
                        {
                            EntityRef ref;
                            ref.list_idx = list_idx;
                            ref.entity_idx = entity_idx;

                            // TODO define this size
                            uint8_t *create_packet_data = new uint8_t[2048];
                            size_t create_packet_size = make_entity_create_packet(ref, entity_manager, INCOMPLETE_ID, create_packet_data, 2048);

                            send_game_packet(
                                server.sock,
                                packet.sender,
                                client_id,
                                GamePacketType::ENTITY_CREATE,
                                create_packet_data,
                                create_packet_size);

                            delete[] create_packet_data;
                        }
                    }
                    break;
                }
                case GamePacketType::PLAYER_SPAWN:
                {
                    handle_player_spawn_req(entity_manager, &server, &packet.packet);
                    break;
                }
                case GamePacketType::CONTROL_UPDATE:
                {
                    server.clients[packet.packet.header.sender].inputs =
                        packet.packet.packet_data.control_update.inputs;
                    server.clients[packet.packet.header.sender].sequence =
                        packet.packet.packet_data.control_update.sequence;
                    server.clients[packet.packet.header.sender].received_input = true;
                    server.clients[packet.packet.header.sender].player_entity = packet.packet.packet_data.control_update.inputs.entity;

                    break;
                }
                default:
                // do nothing
                break;
            }
        }

        // update player control
        for (auto client : server.clients)
        {
            if (!client.received_input) continue;

            handle_player_input(entity_manager, client.player_entity, client.inputs, (float)TIMESTEP);

            // Create an entity if the player shot
            if (client.inputs.ship.shoot)
            {
                EntityRef player_ref = entity_manager->entity_table.lookup_entity(client.player_entity);
                EntityHandle player_ship_handle = *(EntityHandle*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM_FOLLOWER);
                EntityRef player_ship = entity_manager->entity_table.lookup_entity(player_ship_handle);
                Transform *ship_transform = (Transform*)entity_manager->lookup_component(player_ship, ComponentType::TRANSFORM);
                if (ship_transform)
                {
                    EntityRef ref = create_projectile(*ship_transform, entity_manager);

                    uint8_t *create_packet_data = new uint8_t[2048];
                    size_t create_packet_size = make_entity_create_packet(ref, entity_manager, INCOMPLETE_ID, create_packet_data, 2048);
                    
                    server.broadcast(
                        GamePacketType::ENTITY_CREATE,
                        create_packet_data,
                        create_packet_size);

                    delete[] create_packet_data;
                }
            }

            client.received_input = false;
        }

        perform_entity_update_step(entity_manager, TIMESTEP);

        // Update transform followers
        for (uint32_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
        {
            EntityListInfo &list = entity_manager->entity_lists[list_idx];
            if (!list.supports_component(ComponentType::TRANSFORM)) continue;
            if (!list.supports_component(ComponentType::WORLD_SECTOR)) continue;
            if (!list.supports_component(ComponentType::TRANSFORM_FOLLOWER)) continue;
            update_transform_followers(entity_manager, (EntityHandle*)list.components[ComponentType::TRANSFORM_FOLLOWER], (Transform*)list.components[ComponentType::TRANSFORM], (WorldSector*)list.components[ComponentType::WORLD_SECTOR], list.size); 
        }

        // Sync all transforms
        {
            EntityRef ref;
            for (ref.list_idx = 0; ref.list_idx < entity_manager->entity_lists.size(); ref.list_idx++)
            {
                EntityListInfo &list = entity_manager->entity_lists[ref.list_idx];
                if (!list.supports_component(ComponentType::TRANSFORM)) continue;

                if (!list.supports_component(ComponentType::WORLD_SECTOR)) continue;

                Transform *transforms = (Transform*)list.components[ComponentType::TRANSFORM];
                WorldSector *position_rfs = (WorldSector*)list.components[ComponentType::WORLD_SECTOR];

                for (ref.entity_idx = 0; ref.entity_idx < list.size; ref.entity_idx++)
                {
                    // Some transforms do not have seq nums, so send zero for those
                    uint32_t seq_num = 0;
                    ClientId sync_client = INCOMPLETE_ID;

                    // Check if this is a player entity:
                    for (uint32_t client_id = 0; client_id < server.clients.size(); client_id++)
                    {
                        ClientConnection client = server.clients[client_id];
                        if (list.handles[ref.entity_idx] == client.player_entity)
                        {
                            seq_num = client.sequence;
                            sync_client = client_id;
                        }
                    }

                    // Send the sync packet
                    TransformSyncPacket transform_sync(
                        list.handles[ref.entity_idx],
                        transforms[ref.entity_idx],
                        position_rfs[ref.entity_idx],
                        seq_num,
                        sync_client);
                    server.broadcast(
                       GamePacketType::PHYSICS_SYNC,
                       &transform_sync,
                       sizeof(transform_sync));
                }
            }
        }

        double delta_time = time_keeper.get_delta_time_s_no_reset();

        // wait until we've hit 60 fps
        double time_remaining = TIMESTEP - delta_time;
        if (time_remaining > 0.0)
        {
            if (time_remaining > 0.001)
            {
                SDL_Delay((uint32_t)(1000 * time_remaining));
            }
            // busy wait for the rest of the time
            delta_time = time_keeper.get_delta_time_s_no_reset();
            while (delta_time < TIMESTEP)
            {
                delta_time = time_keeper.get_delta_time_s_no_reset();
            }
        }

        delta_time = time_keeper.get_delta_time_s();
    }
}
