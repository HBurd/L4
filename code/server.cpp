#include "SDL/SDL.h"
#include <iostream>

#include "hbmath.h"
#include "hbrenderer.h"
#include "hbtime.h"
#include "hbentities.h"
#include "hbship.h"
#include "hbutil.h"
#include "hbkeyboard.h"
#include "hbplayer_control.h"
#include "hbnet.h"
#include "hbgui.h"
#include "hbprojectile.h"
#include "hbentity_update_step.h"

#include <sys/prctl.h>
#include <signal.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

int main(int argc, char* argv[])
{
    // exit when parent exits
    prctl(PR_SET_PDEATHSIG, SIGHUP);

    ServerData server;

    // parse command line arguments
    {
        if (argc != 2)
        {
            cout << "Usage: L4Server [port]" << endl;
            return 1;
        }
        uint32_t port = atoi(argv[1]);
        if (port == 0 || port & 0xFFFF0000)
        {
            cout << "A valid port was not supplied." << endl;
            return 1;
        }

        // initialize the server
        server.init(port);
    }

    vector<GamePacketIn> game_packets;

    EntityManager entity_manager;
    // add list for projectiles
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PROJECTILE));
    // add list for players
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PLAYER_CONTROL));
 
    TimeKeeper time_keeper;
    double delta_time;

    bool running = true;
    while (running)
    {
        perform_entity_update_step(&entity_manager, delta_time);

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
                                entity_list1.player_control_list[entity1_idx].client_id,
                                SERVER_ID);
                            server.broadcast(*(GamePacket*)&player_damage_packet);
                        }
                    }
                }
            }
        }
 
        // Process incoming packets
        get_packets(server.sock, &game_packets);
        for (auto packet : game_packets)
        {
            switch (packet.packet.header.type)
            {
                case GamePacketType::CONNECTION_REQ:
                    cout << "INCOMING CONNECTION" << endl;
                    server.accept_client(packet.sender);
                    // now update the client with all existing entities
                    for (size_t list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
                    {
                        EntityList& entity_list = entity_manager.entity_lists[list_idx];
                        for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
                        {
                            Entity entity = entity_list.serialize(entity_idx);
                            EntityCreatePacket create_packet(
                                entity,
                                entity_list.handles[entity_idx],
                                SERVER_ID);
                            sendto(
                                server.sock,
                                &create_packet,
                                sizeof(create_packet),
                                0,
                                (sockaddr*)&packet.sender,
                                sizeof(packet.sender));
                        }
                    }
                    break;
                case GamePacketType::PLAYER_SPAWN:
                {
                    Entity ship_entity = create_ship(packet.packet.player_spawn.coords);
                    ship_entity.supported_components |= ComponentType::PLAYER_CONTROL;
                    ship_entity.player_control = {packet.packet.header.sender};
                    EntityHandle ship_entity_handle = 
                        entity_manager.create_entity(ship_entity);

                    server.clients[packet.packet.header.sender].player_entity = ship_entity_handle;

                    EntityCreatePacket entity_create_packet(ship_entity, ship_entity_handle, SERVER_ID);
                    
                    server.broadcast(*(GamePacket*)&entity_create_packet);
                    break;
                }
                case GamePacketType::CONTROL_UPDATE:
                {
                    size_t player_list_idx;
                    size_t player_entity_idx;
                    entity_manager.entity_table.lookup_entity(
                        server.clients[packet.packet.header.sender].player_entity,
                        entity_manager.entity_lists,
                        &player_list_idx,
                        &player_entity_idx);

                    player_control_update(
                        &entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx],
                        packet.packet.control_update.state);

                    if (packet.packet.control_update.state.shoot)
                    {
                        Entity projectile_entity = create_projectile(
                            entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx]);
                        EntityHandle projectile_handle =
                            entity_manager.create_entity(projectile_entity);
                        EntityCreatePacket entity_create_packet(
                            projectile_entity,
                            projectile_handle,
                            SERVER_ID);
                        
                        server.broadcast(*(GamePacket*)&entity_create_packet);
                    }

                    PhysicsSyncPacket physics_sync(
                        server.clients[packet.packet.header.sender].player_entity,
                        entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx],
                        SERVER_ID);
                    server.broadcast(*(GamePacket*)&physics_sync);

                    break;
                }
            }
        }

        delta_time = time_keeper.get_delta_time_s();

        // wait until we've hit 60 fps
        double time_remaining = 0.016667 - delta_time;
        if (time_remaining > 0.0)
        {
            if (time_remaining > 0.001)
            {
                SDL_Delay((uint32_t)(1000 * time_remaining));
            }
        }
    }

    SDL_Quit();
}
