#include "SDL/SDL.h"
#include <iostream>
#include <cassert>

#ifdef __unix__
#include <sys/prctl.h>
#include <signal.h>
#endif

#include "hec.h"

#include "hb/math.h"
#include "hb/renderer.h"
#include "hb/time.h"
#include "hb/util.h"
#include "hb/player_control.h"
#include "hb/server.h"
#include "hb/packets.h"
#include "hb/entity_update_step.h"
#include "hb/entity_initializers.h"
#include "hb/physics.h"
#include "hb/components.h"

const double TIMESTEP = 1.0 / 60.0;

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

int main(int argc, char* argv[])
{
#ifdef __unix__
    // TODO: doesn't belong here
    // exit when parent exits
    prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif

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
    for (uint32_t i = 0; i < ARRAY_LENGTH(components); i++)
    {
        switch(i)
        {
            case ComponentType::WORLD_SECTOR:
                components[i] = {sizeof(WorldSector)};
                break;
            case ComponentType::TRANSFORM:
                components[i] = {sizeof(Transform)};
                break;
            case ComponentType::PLAYER_CONTROL:
                components[i] = {sizeof(PlayerControl)};
                break;
            case ComponentType::PLANET:
                components[i] = {sizeof(Planet)};
                break;
            case ComponentType::PHYSICS:
                components[i] = {sizeof(Physics)};
                break;
            case ComponentType::PROJECTILE:
                components[i] = {sizeof(Projectile)};
                break;
            case ComponentType::MESH:
                components[i] = {sizeof(MeshId)};
                break;
            default:
                assert(false);
        }
    }

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
                            size_t create_packet_size = make_entity_create_packet(ref, entity_manager, create_packet_data, 2048);

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
                    EntityRef ref = create_player_ship(packet.packet.packet_data.player_spawn.coords, packet.packet.header.sender, entity_manager);
                    EntityHandle handle = entity_manager->entity_lists[ref.list_idx].handles[ref.entity_idx];

                    server.clients[packet.packet.header.sender].player_entity = handle;

                    uint8_t *create_packet_data = new uint8_t[2048];
                    size_t create_packet_size = make_entity_create_packet(ref, entity_manager, create_packet_data, 2048);
                    
                    server.broadcast(
                        GamePacketType::ENTITY_CREATE,
                        create_packet_data,
                        create_packet_size);

                    delete[] create_packet_data;
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

            EntityRef player_ref;
            entity_manager->entity_table.lookup_entity(client.player_entity, &player_ref);

            Transform &player_transform = *(Transform*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM);
            Physics player_physics = *(Physics*)entity_manager->lookup_component(player_ref, ComponentType::PHYSICS);

            Vec3 ship_thrust;
            Vec3 ship_torque;
            get_ship_thrust(
                client.player_control,
                player_transform.orientation,
                &ship_thrust,
                &ship_torque);

            apply_impulse(
                ship_thrust * TIMESTEP,
                &player_transform.velocity,
                player_physics.mass);
            apply_angular_impulse(
                ship_torque * TIMESTEP,
                &player_transform.angular_velocity,
                player_physics.angular_mass);

            // Create an entity if the player shot
            if (client.player_control.shoot)
            {
                EntityRef ref = create_projectile(player_transform, entity_manager);

                uint8_t *create_packet_data = new uint8_t[2048];
                size_t create_packet_size = make_entity_create_packet(ref, entity_manager, create_packet_data, 2048);
                
                server.broadcast(
                    GamePacketType::ENTITY_CREATE,
                    create_packet_data,
                    create_packet_size);

                delete[] create_packet_data;
            }

            // Send the sync packet
            TransformSyncPacket transform_sync(
                client.player_entity,
                player_transform,
                client.sequence);
            server.broadcast(
               GamePacketType::PHYSICS_SYNC,
               &transform_sync,
               sizeof(transform_sync));

            client.received_input = false;
        }

        perform_entity_update_step(entity_manager, TIMESTEP);

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
