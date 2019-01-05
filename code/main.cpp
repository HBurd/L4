#include "SDL/SDL.h"
#include <iostream>

#include "unistd.h"

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
#include "hbpackets.h"

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

int main(int argc, char *argv[], char *envp[])
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
 
    //ShaderProgramId skybox_shader_prog = renderer.load_shader("skybox.vert", "skybox.frag");
    //renderer.load_skybox("skybox.obj", "skymap3.png", skybox_shader_prog);

    // Initialize ImGui
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        const char* glsl_version = "#version 330";
        ImGui_ImplSDL2_InitForOpenGL(window, gl_ctxt);
        ImGui_ImplOpenGL3_Init(glsl_version);
        ImGui::StyleColorsDark();
    }

    Keyboard kb;
    
    ClientData client;

    MainMenu main_menu;
    SpawnMenu spawn_menu;
    Console ship_console("Ship Console");
    Console server_console("Server Console");
    bool enable_ui = true;

    struct ClientState
    {
        enum ClientStatus
        {
            DISCONNECTED,
            NOT_SPAWNED,
            SPAWNED
        } status = DISCONNECTED;
        bool server_proc_connected = false;

        EntityHandle player_handle;
        unsigned int player_health = 3;

        EntityHandle guidance_target;
        bool track = false;
        bool stabilize = false;
    } client_state;

    vector<GamePacketIn> game_packets;

    PlayerInputBuffer past_inputs;

    EntityManager entity_manager;
    // add list for projectiles
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PROJECTILE));
    // add list for players
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PLAYER_CONTROL));
 
    TimeKeeper time_keeper;
    double delta_time = 0.0f;

    bool running = true;
    while (running)
    {
        kb.clear_keydowns();

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
                kb.handle_keydown(event.key.keysym.sym);
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
            if (client_state.status == ClientState::DISCONNECTED)
            {
                bool create_server = false;
                bool connect_to_server = false;

                main_menu.draw(&create_server, &connect_to_server);

                if (create_server)
                {
                    client.create_server(main_menu.port);
                    // connect to localhost
                    // TODO: need a better sln
                    usleep(100000);
                    client.connect(0x7f000001, main_menu.port);
                    client_state.server_proc_connected = true;
                    client_state.status = ClientState::NOT_SPAWNED;
                }
                else if (connect_to_server)
                {
                    client.connect(main_menu.ip, main_menu.port);
                    client_state.status = ClientState::NOT_SPAWNED;
                }
            }

            if (client_state.status == ClientState::NOT_SPAWNED && spawn_menu.draw())
            {
                client.spawn(spawn_menu.coords);
            }

            if (client_state.status == ClientState::SPAWNED)
            {
                draw_guidance_menu(
                    &entity_manager,
                    client_state.player_handle,
                    &client_state.guidance_target,
                    &client_state.track,
                    &client_state.stabilize);
                ship_console.draw();
            }
            
            if (client_state.server_proc_connected)
            {
                client.write_server_stdout(&server_console);
                server_console.draw();
            }
        }

        // PlayerControl updates
        if (client_state.status == ClientState::SPAWNED)
        {
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

            PlayerControlState control_state = player_control_get_state(
                kb,
                client_state.stabilize,
                player_physics,
                client_state.track,
                target_physics);
            control_state.shoot = kb.down.enter;

            handle_player_input(
                control_state,
                delta_time,
                &player_physics,
                &past_inputs,
                &client);

            /*

            ControlUpdatePacket control_update(control_state, client_state.id);
            client.send_to_server(*(GamePacket*)&control_update);

            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            
            // client side prediction:
            player_control_update(
                &player_physics,
                control_state,
                delta_time);
                */
        }

        perform_entity_update_step(&entity_manager, delta_time);

        // Process incoming packets
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
                        && packet.packet.entity_create.entity.player_control.client_id == client.id)
                    {
                        assert(client_state.status == ClientState::NOT_SPAWNED);
                        client_state.status = ClientState::SPAWNED;
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
                //case GamePacketType::PLAYER_DAMAGE:
                //{
                //    if (packet.packet.player_damage.player == client_state.id)
                //    {
                //        //// TODO very bad
                //        //client_state.player_health--;
                //        //char damage_string[] = 
                //        //    "TAKING DAMAGE!!\n"
                //        //    "Ship health is 3/3\n";
                //        //damage_string[31] = '0' + client_state.player_health;
                //        //ship_console.write(damage_string);
                //    }
                //    break;
                //}
            }
        }

        // Update camera
        if (client_state.status == ClientState::SPAWNED)
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
