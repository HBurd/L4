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
    MeshId projectile_mesh = renderer.load_mesh("projectile.obj", ship_shader_prog);

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

    MainMenu main_menu;
    SpawnMenu spawn_menu;
    ShipConsole ship_console;
    bool enable_ui = true;

    struct ClientState
    {
        ClientId id = INCOMPLETE_ID;
        bool has_spawned = false;
        EntityHandle player_handle;
        PlayerControlState control_state;
        bool shoot = false;
        unsigned int player_health = 3;

        EntityHandle guidance_target;
        bool track = false;
        bool stabilize = false;
    } client_state;

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
                // ====================================
                // vv HANDLE ONESHOT KEYPRESSES HERE vv
                // ====================================
                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    enable_ui = !enable_ui;
                }
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    client_state.shoot = true;
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

        if (enable_ui)
        {
            bool create_server = false;
            bool connect_to_server = false;
            main_menu.draw(&create_server, &connect_to_server);

            if (create_server)
            {
                client_state.id = server.init(main_menu.port);
                main_menu.draw_main_menu = false;
                spawn_menu.draw_spawn_menu = true;
            }
            else if (connect_to_server)
            {
                client_state.id = client.connect(main_menu.ip, main_menu.port);
                main_menu.draw_main_menu = false;
                spawn_menu.draw_spawn_menu = true;
            }

            if (spawn_menu.draw(client_state.id != SERVER_ID) && !client_state.has_spawned)
            {
                client.spawn(spawn_menu.coords);
                spawn_menu.draw_spawn_menu = false;
            }

            if (client_state.has_spawned)
            {
                draw_guidance_menu(
                    &entity_manager,
                    client_state.player_handle,
                    &client_state.guidance_target,
                    &client_state.track,
                    &client_state.stabilize);

                ship_console.draw();
            }
        }
        

        // ==============
        // Entity Updates
        // ==============

        // PlayerControl updates
        if (client_state.has_spawned)
        {
            // TODO: Handle server case
            if (client_state.id == SERVER_ID) continue;

            // look up the player entity
            size_t list_idx;
            size_t entity_idx;
            entity_manager.entity_table.lookup_entity(
                client_state.player_handle,
                entity_manager.entity_lists,
                &list_idx,
                &entity_idx);

            Physics& player_physics = entity_manager.entity_lists[list_idx].physics_list[entity_idx];
            
            Physics target_physics;
            if (client_state.track)
            {
                size_t target_list_idx;
                size_t target_entity_idx;
                entity_manager.entity_table.lookup_entity(
                    client_state.guidance_target,
                    entity_manager.entity_lists,
                    &target_list_idx,
                    &target_entity_idx);
                
                target_physics = entity_manager.entity_lists[target_list_idx].physics_list[target_entity_idx];
            }

            client_state.control_state = player_control_get_state(
                kb,
                client_state.stabilize,
                player_physics,
                client_state.track,
                target_physics);
            client_state.control_state.shoot = client_state.shoot;
            client_state.shoot = false;

            ControlUpdatePacket control_update(client_state.control_state, client_state.id);
            client.send_to_server(*(GamePacket*)&control_update);

            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            
            // client side prediction:
            player_control_update(
                &player_physics,
                client_state.control_state);
        }

        // Physics updates
        for (size_t list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            if (!entity_list.supports_components(ComponentType::PHYSICS))
                continue;
            for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                Physics& physics = entity_list.physics_list[entity_idx];
                physics.position += physics.velocity;
                physics.orientation = physics.orientation * physics.angular_velocity;
                
                // we need to normalize orientation and angular velocity every frame,
                // or we get accumulating floating point errors
                physics.orientation = physics.orientation.normalize();
                physics.angular_velocity = physics.angular_velocity.normalize();
            }
        }

        // Projectile updates
        for (size_t list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            if (!entity_list.supports_components(ComponentType::PROJECTILE))
                continue;
            for (size_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                if (!projectile_update(&entity_list.projectile_list[entity_idx]))
                {
                    // TODO: synchronize between clients and server?
                    // also will this break the loops?
                    entity_manager.kill_entity(entity_list.handles[entity_idx]);
                }
            }
        }

        // check for collisions
        if (server.active)
        {
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
                                    client_state.id);
                                server.broadcast(*(GamePacket*)&player_damage_packet);
                            }
                        }
                    }
                }
            }
        }
 
        // Process incoming packets
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
                                    client_state.id);
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

                        EntityCreatePacket entity_create_packet(ship_entity, ship_entity_handle, client_state.id);
                        
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
                                entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx],
                                projectile_mesh);
                            EntityHandle projectile_handle =
                                entity_manager.create_entity(projectile_entity);
                            EntityCreatePacket entity_create_packet(
                                projectile_entity,
                                projectile_handle,
                                client_state.id);
                            
                            server.broadcast(*(GamePacket*)&entity_create_packet);
                        }

                        PhysicsSyncPacket physics_sync(
                            server.clients[packet.packet.header.sender].player_entity,
                            entity_manager.entity_lists[player_list_idx].physics_list[player_entity_idx],
                            client_state.id);
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
                        // check if created entity is the player entity
                        if ((packet.packet.entity_create.entity.supported_components
                             & ComponentType::PLAYER_CONTROL)
                            && packet.packet.entity_create.entity.player_control.client_id == client_state.id)
                        {
                            assert(!client_state.has_spawned);
                            client_state.has_spawned = true;
                            client_state.player_handle = packet.packet.entity_create.handle;
                            ship_console.write(
                                "Welcome to your new AN-111 spaceship!\n"
                                "The buttons marked Q, W, E, A, S and D in the\n"
                                "cockpit will control your orientation.\n"
                                "The lever marked SPACE controls forward thrust.\n"
                                "Weapons can be triggered by pressing ENTER on\n"
                                "the weapons control panel.\n"
                                "...\n");
                        }
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
                    case GamePacketType::PLAYER_DAMAGE:
                    {
                        if (packet.packet.player_damage.player == client_state.id)
                        {
                            //// TODO very bad
                            //client_state.player_health--;
                            //char damage_string[] = 
                            //    "TAKING DAMAGE!!\n"
                            //    "Ship health is 3/3\n";
                            //damage_string[31] = '0' + client_state.player_health;
                            //ship_console.write(damage_string);
                        }
                        break;
                    }
                }
            }
        }

        // Update camera
        if (client_state.has_spawned)
        {
            size_t list_idx;
            size_t entity_idx;
            if(entity_manager.entity_table.lookup_entity(
                   client_state.player_handle,
                   entity_manager.entity_lists,
                   &list_idx,
                   &entity_idx))
            {
                renderer.camera_pos = entity_manager.entity_lists[list_idx].physics_list[entity_idx].position;
                renderer.camera_orientation =
                    entity_manager.entity_lists[list_idx].physics_list[entity_idx].orientation;
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
