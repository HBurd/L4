#include "SDL/SDL.h"
#include <iostream>

#include <sys/prctl.h>
#include <signal.h>

#include "hb/math.h"
#include "hb/renderer.h"
#include "hb/time.h"
#include "hb/entities.h"
#include "hb/ship.h"
#include "hb/util.h"
#include "hb/player_control.h"
#include "hb/server.h"
#include "hb/packets.h"
#include "hb/projectile.h"
#include "hb/entity_update_step.h"

const double TIMESTEP = 1.0 / 60.0;

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

int main(int argc, char* argv[])
{
    // exit when parent exits
    prctl(PR_SET_PDEATHSIG, SIGHUP);

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

    EntityManager entity_manager;
    // add list for projectiles
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PROJECTILE));
    // add list for players
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PLAYER_CONTROL));
 
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
                    cout << "Client connected with id " << client_id << endl;
                    // now update the client with all existing entities
                    for (size_t list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
                    {
                        EntityList& entity_list = entity_manager.entity_lists[list_idx];
                        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
                        {
                            Entity entity = entity_list.serialize(entity_idx);
                            EntityCreatePacket create_packet(
                                entity,
                                entity_list.handles[entity_idx]);
                            send_game_packet(
                                server.sock,
                                packet.sender,
                                client_id,
                                GamePacketType::ENTITY_CREATE,
                                &create_packet,
                                sizeof(create_packet));
                        }
                    }
                    break;
                }
                case GamePacketType::PLAYER_SPAWN:
                {
                    Entity ship_entity = create_ship(packet.packet.packet_data.player_spawn.coords);
                    ship_entity.supported_components |= ComponentType::PLAYER_CONTROL;
                    ship_entity.player_control = {packet.packet.header.sender};
                    EntityHandle ship_entity_handle = 
                        entity_manager.create_entity(ship_entity);

                    server.clients[packet.packet.header.sender].player_entity = ship_entity_handle;

                    EntityCreatePacket entity_create_packet(ship_entity, ship_entity_handle);
                    
                    server.broadcast(
                        GamePacketType::ENTITY_CREATE,
                        &entity_create_packet,
                        sizeof(entity_create_packet));

                    break;
                }
                case GamePacketType::CONTROL_UPDATE:
                {
                    server.clients[packet.packet.header.sender].player_control =
                        packet.packet.packet_data.control_update.state;
                    server.clients[packet.packet.header.sender].sequence =
                        packet.packet.packet_data.control_update.sequence;
                    server.clients[packet.packet.header.sender].received_input = true;

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
            // look up player entity
            size_t list_idx;
            size_t entity_idx;
            if (entity_manager.entity_table.lookup_entity(
                    client.player_entity,
                    entity_manager.entity_lists,
                    &list_idx,
                    &entity_idx))
            { 
                Physics* player_physics = &entity_manager.entity_lists[list_idx].physics_list[entity_idx];
                player_control_update(player_physics, client.player_control, TIMESTEP);

                // Create an entity if the player shot
                if (client.player_control.shoot)
                {
                    Entity projectile_entity = create_projectile(
                        entity_manager.entity_lists[list_idx].physics_list[entity_idx]);
                    EntityHandle projectile_handle =
                        entity_manager.create_entity(projectile_entity);
                    EntityCreatePacket entity_create_packet(
                        projectile_entity,
                        projectile_handle);
                    
                    server.broadcast(
                        GamePacketType::ENTITY_CREATE,
                        &entity_create_packet,
                        sizeof(entity_create_packet));
                }

                // Send the sync packet
                PhysicsSyncPacket physics_sync(
                    client.player_entity,
                    entity_manager.entity_lists[list_idx].physics_list[entity_idx],
                    client.sequence);
                server.broadcast(
                   GamePacketType::PHYSICS_SYNC,
                   &physics_sync,
                   sizeof(physics_sync));
            }

            client.received_input = false;
        }

        perform_entity_update_step(&entity_manager, TIMESTEP);

        /*
        // check for collisions
        for (size_t list1_idx = 0; list1_idx < entity_manager.entity_lists.size(); list1_idx++)
        {
            EntityList& entity_list1 = entity_manager.entity_lists[list1_idx];
            if (!entity_list1.supports_components(ComponentType::PLAYER_CONTROL | ComponentType::PHYSICS))
                continue;
            for (size_t entity1_idx = 0; entity1_idx < entity_list1.size; entity1_idx++)
            {
                for (size_t list2_idx = 0; list2_idx < entity_manager.entity_lists.size(); list2_idx++)
                {
                    EntityList& entity_list2 = entity_manager.entity_lists[list2_idx];
                    if (!entity_list2.supports_components(ComponentType::PHYSICS))
                        continue;
                    for (size_t entity2_idx = 0; entity2_idx < entity_list2.size; entity2_idx++)
                    {
                        if (list1_idx == list2_idx && entity1_idx == entity2_idx)
                            continue;
                        const float collision_distance = 1.0f;
                        if ((entity_list1.physics_list[entity1_idx].position
                             - entity_list2.physics_list[entity2_idx].position)
                             .norm() < collision_distance)
                        {
                            PlayerDamagePacket player_damage_packet(
                                entity_list1.player_control_list[entity1_idx].client_id);
                            server.broadcast(*(GamePacket*)&player_damage_packet);
                        }
                    }
                }
            }
        }
        */
 
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

    SDL_Quit();
}
