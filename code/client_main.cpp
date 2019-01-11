#include "SDL/SDL.h"
#include <iostream>

#include "hb/math.h"
#include "hb/renderer.h"
#include "hb/time.h"
#include "hb/entities.h"
#include "hb/ship.h"
#include "hb/util.h"
#include "hb/keyboard.h"
#include "hb/player_control.h"
#include "hb/net.h"
#include "hb/gui.h"
#include "hb/projectile.h"
#include "hb/entity_update_step.h"
#include "hb/packets.h"
#include "hb/handle_player_input.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;

const double TIMESTEP = 1.0 / 60.0;

const int INITIAL_WINDOW_WIDTH = 800;
const int INITIAL_WINDOW_HEIGHT = 600;

int main(int argc, char *argv[])
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

        EntityHandle guidance_target;
        bool track = false;
        bool stabilize = false;
    } client_state;

    // check if a server address was supplied
    if (argc == 3)
    {
        client.connect(parse_ip4(argv[1]), atoi(argv[2]));
        client_state.status = ClientState::NOT_SPAWNED;
    }

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
                    client.connect(parse_ip4(main_menu.ip), main_menu.port);
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
                TIMESTEP,
                &player_physics,
                &past_inputs,
                &client);
        }

        perform_entity_update_step(&entity_manager, TIMESTEP);

        // Process incoming packets
        get_packets(client.sock, &game_packets);
        for (auto packet : game_packets)
        {
            switch (packet.packet.header.type)
            {
                case GamePacketType::ENTITY_CREATE:
                    entity_manager.create_entity_with_handle(
                        packet.packet.packet_data.entity_create.entity,
                        packet.packet.packet_data.entity_create.handle);
                    // check if created entity is the player entity
                    if ((packet.packet.packet_data.entity_create.entity.supported_components
                         & ComponentType::PLAYER_CONTROL)
                        && packet.packet.packet_data.entity_create.entity.player_control.client_id == client.id)
                    {
                        assert(client_state.status == ClientState::NOT_SPAWNED);
                        client_state.status = ClientState::SPAWNED;
                        client_state.player_handle = packet.packet.packet_data.entity_create.handle;
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
                            packet.packet.packet_data.physics_sync.entity,
                            entity_manager.entity_lists,
                            &list_idx,
                            &entity_idx))
                    {
                        entity_manager.entity_lists[list_idx].physics_list[entity_idx] = 
                            packet.packet.packet_data.physics_sync.physics_state;
                        if (entity_manager.entity_lists[list_idx].handles[entity_idx]
                                == client_state.player_handle)
                        {
                            // apply later inputs (reconciliation)
                            for (uint32_t sequence = packet.packet.packet_data.physics_sync.sequence;
                                 sequence < past_inputs.next_seq_num;
                                 sequence++)
                            {
                                uint32_t input_idx = sequence % ARRAY_LENGTH(past_inputs.inputs);
                                if (past_inputs.inputs[input_idx].sequence_number == sequence)
                                {
                                    player_control_update(
                                        &entity_manager.entity_lists[list_idx].physics_list[entity_idx],
                                        past_inputs.inputs[input_idx].input,
                                        past_inputs.inputs[input_idx].dt);
                                }
                            }
                        }
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
                default:
                // do nothing
                break;
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
    }

    SDL_Quit();
}
