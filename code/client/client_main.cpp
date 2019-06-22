#include "SDL/SDL.h"
#undef main

#include <iostream>

#include "hec.h"

#include "common/math.h"
#include "common/renderer.h"
#include "common/time.h"
#include "common/util.h"
#include "client/player_input.h"
#include "common/net.h"
#include "common/projectile.h"
#include "common/entity_update_step.h"
#include "common/packets.h"
#include "common/entity_initializers.h"
#include "common/physics.h"
#include "common/components.h"
#include "common/TransformFollowerComponent.h"
#include "common/PlayerControlComponent.h"

#include "client/keyboard.h"
#include "client/gui.h"
#include "client/client.h"
#include "client/client_logic.h"

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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
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

    } client_state;

    // check if a server address was supplied
    if (argc == 3)
    {
        if(client.connect(parse_ip4(argv[1]), atoi(argv[2])))
        {
            client_state.status = ClientState::NOT_SPAWNED;
        }
        else
        {
            cout << "Unable to connect to server" << endl;
        }
    }

    vector<GamePacketIn> game_packets;

    PlayerInputBuffer past_inputs;

    ComponentInfo components[ComponentType::NUM_COMPONENT_TYPES];
    init_component_info(components, ARRAY_LENGTH(components));

    EntityManager *entity_manager =
        new EntityManager(
            components,
            ARRAY_LENGTH(components));

    LocalGameData game(entity_manager);
 
    TimeKeeper time_keeper;

    bool running = true;
    while (running)
    {
        game.input.keyboard.clear_keydowns();

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
                    game.input.keyboard.handle_keyup(event.key.keysym.sym);
                    break;
                case SDL_KEYDOWN:
                    game.input.keyboard.handle_keydown(event.key.keysym.sym);
                    break;
            }
        }

        // Mouse wrapping
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
        {
            int new_x, new_y;
            SDL_GetGlobalMouseState(&new_x, &new_y);
            int window_x, window_y;
            SDL_GetWindowPosition(window, &window_x, &window_y);
            new_x -= window_x;
            new_y -= window_y;
            game.input.mouse.dx = new_x - game.input.mouse.x;
            game.input.mouse.dy = new_y - game.input.mouse.y;
            game.input.mouse.x = new_x;
            game.input.mouse.y = new_y;

            if (game.input.mouse.x >= renderer.width - 1) // 1 pixel extra for when screen is max
            {
                game.input.mouse.x -= renderer.width - 2;
                SDL_WarpMouseInWindow(window, game.input.mouse.x, game.input.mouse.y);
            }
            else if (game.input.mouse.x <= 0)
            {
                game.input.mouse.x += renderer.width - 2;
                SDL_WarpMouseInWindow(window, game.input.mouse.x, game.input.mouse.y);
            }
            if (game.input.mouse.y >= renderer.height - 1)
            {
                game.input.mouse.y -= renderer.height - 2;
                SDL_WarpMouseInWindow(window, game.input.mouse.x, game.input.mouse.y);
            }
            else if (game.input.mouse.y <= 0)
            {
                game.input.mouse.y += renderer.height - 2;
                SDL_WarpMouseInWindow(window, game.input.mouse.x, game.input.mouse.y);
            }
        }

        game.dt = (float)TIMESTEP;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Process incoming packets
        get_packets(client.sock, &game_packets);
        for (auto packet : game_packets)
        {
            switch (packet.packet.header.type)
            {
                case GamePacketType::ENTITY_CREATE:
                    handle_entity_create(&game, &client, packet);
                    break;
                case GamePacketType::PHYSICS_SYNC:
                    handle_physics_sync(&game, &past_inputs, packet);
                    break;
                default:
                    // do nothing
                    break;
            }
        }
        
        if (client_state.status == ClientState::NOT_SPAWNED)
        {
            if (game.player_handle.is_valid())
            {
                client_state.status = ClientState::SPAWNED;
                ship_console.write(
                    "Welcome to your new AN-111 spaceship!\n"
                    "The buttons marked Q, W, E, A, S and D in the\n"
                    "cockpit will control your orientation.\n"
                    "The lever marked SPACE controls forward thrust.\n"
                    "Weapons can be triggered by pressing ENTER on\n"
                    "the weapons control panel.\n"
                    "...\n");
            }
        }

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
                    client.connect(0x7f000001, main_menu.port);
                    client_state.server_proc_connected = true;
                    client_state.status = ClientState::NOT_SPAWNED;
                }
                else if (connect_to_server)
                {
                    if (client.connect(parse_ip4(main_menu.ip), main_menu.port))
                    {
                        client_state.status = ClientState::NOT_SPAWNED;
                    }
                    else
                    {
                        cout << "Unable to connect to server" << endl;
                    }
                }
            }

            if (client_state.status == ClientState::NOT_SPAWNED && spawn_menu.draw())
            {
                client.spawn(spawn_menu.coords);
            }

            if (client_state.status == ClientState::SPAWNED)
            {
                draw_guidance_menu(
                    entity_manager,
                    game.player_handle,
                    &game.tracking);
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
            PlayerInputs player_inputs = process_player_inputs(game);

            ImGui::Begin("Debug");
            if (ImGui::Button("Detach"))
            {
                player_inputs.leave_command_chair = true;
            }
            ImGui::End();

            handle_player_input(entity_manager, game.player_handle, player_inputs, game.dt);

            // Send the control packet
            ControlUpdatePacket control_update(player_inputs, past_inputs.next_seq_num);
            client.send_to_server(GamePacketType::CONTROL_UPDATE, &control_update, sizeof(control_update));

            // Save input for client-side prediction
            past_inputs.save_input(player_inputs, (float)TIMESTEP);
        }

        perform_entity_update_step(entity_manager, (float)TIMESTEP);

        // Update transform followers
        for (uint32_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
        {
            EntityListInfo &list = entity_manager->entity_lists[list_idx];
            if (!list.supports_component(ComponentType::TRANSFORM)) continue;
            if (!list.supports_component(ComponentType::TRANSFORM_FOLLOWER)) continue;
            update_transform_followers(entity_manager, (EntityHandle*)list.components[ComponentType::TRANSFORM_FOLLOWER], (Transform*)list.components[ComponentType::TRANSFORM], list.size); 
        }

        // Update camera
        if (client_state.status == ClientState::SPAWNED)
        {
            EntityRef player_ref = entity_manager->entity_table.lookup_entity(game.player_handle);
            assert(player_ref.is_valid());
            WorldSector world_sector = *(WorldSector*)entity_manager->lookup_component(player_ref, ComponentType::WORLD_SECTOR);
            Transform transform = *(Transform*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM);

            renderer.camera_pos = to_world_position(world_sector, transform.position);
            renderer.camera_orientation = transform.orientation;
        }
        
        // =========
        // Rendering
        // =========

        renderer.clear();

        renderer.draw_skybox();

        for (unsigned int list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
        {
            EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
            // check the list has suitable components for rendering
            if (!entity_list.supports_component(ComponentType::WORLD_SECTOR))
                continue;
            if (!entity_list.supports_component(ComponentType::TRANSFORM))
                continue;
            if (!entity_list.supports_component(ComponentType::MESH))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                EntityRef ref;
                ref.list_idx = list_idx;
                ref.entity_idx = entity_idx;

                MeshId mesh = *(MeshId*)entity_manager->lookup_component(ref, ComponentType::MESH);
                WorldSector world_sector = *(WorldSector*)entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR);
                Transform transform = *(Transform*)entity_manager->lookup_component(ref, ComponentType::TRANSFORM);
                renderer.draw_mesh(
                    mesh,
                    to_world_position(world_sector, transform.position),
                    transform.scale,
                    transform.orientation);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);

        double delta_time = time_keeper.get_delta_time_s_no_reset();

        // wait until we've hit 60 fps
        double time_remaining = TIMESTEP - delta_time;
        if (time_remaining > 0.0)
        {
            if (time_remaining > 0.001)
            {
                SDL_Delay((uint32_t)(1000 * floor(time_remaining)));
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

    return 0;
}
