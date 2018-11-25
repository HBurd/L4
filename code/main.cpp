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

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

const int INITIAL_WINDOW_WIDTH = 800;
const int INITIAL_WINDOW_HEIGHT = 600;

int main()
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "L4",
        20, 20, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GLContext gl_ctxt = SDL_GL_CreateContext(window);

    Renderer renderer(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
 
    ShaderProgramId ship_shader_prog = renderer.load_shader("triangle.vert", "triangle.frag");
    MeshId ship_mesh = renderer.load_mesh("ship.obj", ship_shader_prog);

    ShaderProgramId skybox_shader_prog = renderer.load_shader("skybox.vert", "skybox.frag");
    renderer.load_skybox("skybox.obj", "skymap3.png", skybox_shader_prog);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    const char* glsl_version = "#version 330";
    ImGui_ImplSDL2_InitForOpenGL(window, gl_ctxt);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    Keyboard kb;
    // NetworkInterface network;
    
    ClientData client;
    ServerData server;
    NetworkGui network_gui;
    ClientId client_id = INCOMPLETE_ID;

    vector<GamePacketIn> game_packets;

    EntityManager entity_manager;
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH));
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PLAYER_CONTROL));

    // add test entities
    //EntityHandle test_entity1;
    //{
    //    Entity player_ship_entity = create_ship(Vec3(-1.5f, 0.0f, -3.0f), ship_mesh);
    //    player_ship_entity.supported_components |= ComponentType::PLAYER_CONTROL;
    //    EntityHandle test_entity1 = entity_manager.create_entity(player_ship_entity);
    //    EntityHandle test_entity2 = entity_manager.create_entity(
    //        create_ship(Vec3(1.5f, 0.0f, -3.0f), ship_mesh));
    //    EntityHandle test_entity3 = entity_manager.create_entity(
    //        create_ship(Vec3(0.0f, 0.0f, 2.0f), ship_mesh));
    //}
 
    bool show_frame_rate = true;

    TimeKeeper time_keeper;
    double delta_time;

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                {
                    unsigned int new_width, new_height;
                    SDL_GetWindowSize(
                        window, 
                        (int*)&new_width,
                        (int*)&new_height);
                    renderer.set_screen_size(new_width, new_height);
                    break;
                }
            case SDL_KEYUP:
                kb.handle_keyup(event.key.keysym.sym);
                break;
            case SDL_KEYDOWN:
                kb.handle_keydown( event.key.keysym.sym);
                // handle oneshot keypresses here
                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    network_gui.main_gui = !network_gui.main_gui;
                }
                break;
            }
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (show_frame_rate)
        {
            ImGui::Begin(
                "Frame Rate",
                &show_frame_rate,
                ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove);
            ImGui::Text("%d fps", (unsigned int)(1.0 / delta_time + 0.5));
            ImGui::End();
        }

        {
            bool create_server = false;
            bool connect_to_server = false;
            network_gui.draw(&create_server, &connect_to_server);

            if (create_server)
            {
                client_id = server.init(network_gui.port);
            }
            else if (connect_to_server)
            {
                client_id = client.connect(network_gui.ip, network_gui.port);
                client.spawn(Vec3(0.0f, 0.0f, -3.0f));
            }
        }

        // ==============
        // Entity Updates
        // ==============
        
        // Physics updates
        for (unsigned int list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            if (!entity_list.supports_components(ComponentType::PHYSICS))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                entity_list.physics_list[entity_idx].position += entity_list.physics_list[entity_idx].velocity;
                entity_list.physics_list[entity_idx].orientation =
                    entity_list.physics_list[entity_idx].orientation
                    * entity_list.physics_list[entity_idx].angular_velocity;
            }
        }

        // PlayerControl updates
        // TODO: we should just cache the player entity when we get it
        for (unsigned int list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            if (!entity_list.supports_components(ComponentType::PLAYER_CONTROL | ComponentType::PHYSICS))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                if (entity_list.player_control_list[entity_idx].client_id != client_id)
                    continue;
                PlayerControlState control_state = player_control_get_state(kb);

                // TODO: Handle server case
                if (client_id == SERVER_ID) continue;

                ControlUpdatePacket control_update(control_state, client_id);
                client.send_to_server(*(GamePacket*)&control_update);
                
                // client side prediction:
                player_control_update(
                    &entity_manager.entity_lists[list_idx].physics_list[entity_idx],
                    control_state);

                // also update camera here
                renderer.camera_pos = entity_list.physics_list[entity_idx].position;
                renderer.camera_orientation = entity_list.physics_list[entity_idx].orientation;
            }
        }

        if (server.active)
        {
            get_packets(server.sock, &game_packets);
            for (auto packet : game_packets)
            {
                switch (packet.packet.header.type)
                {
                    case GamePacketType::CONNECTION_REQ:
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
                                    client_id);
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
                        Entity ship_entity = create_ship(packet.packet.player_spawn.coords, ship_mesh);
                        ship_entity.supported_components |= ComponentType::PLAYER_CONTROL;
                        ship_entity.player_control = {packet.packet.header.sender};
                        EntityHandle ship_entity_handle = 
                            entity_manager.create_entity(ship_entity);

                        server.clients[packet.packet.header.sender].player_entity = ship_entity_handle;

                        EntityCreatePacket entity_create_packet(ship_entity, ship_entity_handle, client_id);
                        
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

                        PhysicsSyncPacket physics_sync(
                            server.clients[packet.packet.header.sender].player_entity,
                            entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx],
                            client_id);
                        server.broadcast(*(GamePacket*)&physics_sync);

                        break;
                    }
                }
            }
        }
        else if (client.active)
        {
            get_packets(client.sock, &game_packets);
            for (auto packet : game_packets)
            {
                switch (packet.packet.header.type)
                {
                    case GamePacketType::ENTITY_CREATE:
                        entity_manager.create_entity_with_handle(
                            packet.packet.entity_create.entity,
                            packet.packet.entity_create.handle);
                        break;
                    case GamePacketType::PHYSICS_SYNC:
                    {
                        size_t list_idx;
                        size_t entity_idx;
                        if (entity_manager.entity_table.lookup_entity(
                                packet.packet.physics_sync.entity,
                                entity_manager.entity_lists,
                                &list_idx,
                                &entity_idx))
                        {
                            entity_manager.entity_lists[list_idx].physics_list[entity_idx] = 
                                packet.packet.physics_sync.physics_state;
                        }
                        break;
                    }
                }
            }
        }
        
        // =========
        // Rendering
        // =========

        renderer.clear();

        renderer.draw_skybox();

        for (unsigned int list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            // check the list has suitable components for rendering
            if (!entity_list.supports_components(ComponentType::PHYSICS | ComponentType::MESH))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                MeshId ship_mesh = entity_list.mesh_list[entity_idx];
                Physics ship_physics = entity_list.physics_list[entity_idx];
                renderer.draw_mesh(ship_mesh, ship_physics.position, ship_physics.orientation);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

        delta_time = time_keeper.get_delta_time_s();

        //// wait until we've hit 60 fps
        //double time_remaining = 0.016667 - delta_time;
        //if (time_remaining > 0.0)
        //{
        //    if (time_remaining > 0.001)
        //    {
        //        SDL_Delay(
        //    }

        //    while (time_remaining 
        //}
    }

    SDL_Quit();
}
