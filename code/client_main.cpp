#include "SDL/SDL.h"
#undef main

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
#include "hb/client.h"

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

    EntityManager *entity_manager = new EntityManager();
 
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
            entity_manager->entity_table.lookup_entity(
                client_state.player_handle,
                entity_manager->entity_lists,
                &list_idx,
                &entity_idx);

            Transform &player_transform =
                entity_manager->entity_lists[list_idx].transform_list[entity_idx];
            Physics &player_physics =
                entity_manager->entity_lists[list_idx].physics_list[entity_idx];

            PlayerControlState control_state;
            
            if (client_state.track)
            {
                // Target tracking is enabled, overriding other control methods
                size_t target_list_idx;
                size_t target_entity_idx;
                if (entity_manager->entity_table.lookup_entity(
                    client_state.guidance_target,
                    entity_manager->entity_lists,
                    &target_list_idx,
                    &target_entity_idx))
                {
                    Transform target_transform =
                        entity_manager
                            ->entity_lists[target_list_idx]
                            .transform_list[target_entity_idx];
                    control_state.torque += compute_target_tracking_torque(
                        player_transform,
                        player_physics,
                        target_transform);
                }
            }
            else if (client_state.stabilize)
            {
                control_state.torque += compute_stabilization_torque(
                    player_transform,
                    player_physics);
            }

            // The player always has some control even when some controller is acting
            control_state.torque += compute_player_input_torque(kb);
            control_state.thrust += compute_player_input_thrust(kb);

            control_state.clamp();

            control_state.shoot = kb.down.enter;

            ControlUpdatePacket control_update(control_state, past_inputs.next_seq_num);
            client.send_to_server(GamePacketType::CONTROL_UPDATE, &control_update, sizeof(control_update));

            past_inputs.save_input(control_state, (float)TIMESTEP);

            Vec3 ship_thrust;
            Vec3 ship_torque;
            get_ship_thrust(
                control_state,
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
        }

        perform_entity_update_step(entity_manager, (float)TIMESTEP);

        // Process incoming packets
        get_packets(client.sock, &game_packets);
        for (auto packet : game_packets)
        {
            switch (packet.packet.header.type)
            {
                case GamePacketType::ENTITY_CREATE:
                    entity_manager->create_entity_with_handle(
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
                    if (entity_manager->entity_table.lookup_entity(
                            packet.packet.packet_data.transform_sync.entity,
                            entity_manager->entity_lists,
                            &list_idx,
                            &entity_idx))
                    {
                        EntityList &list = entity_manager->entity_lists[list_idx];
                        if (list.handles[entity_idx] == client_state.player_handle)
                        {
                            // if we have received no later transform syncs from the server
                            if (packet.packet.packet_data.transform_sync.sequence
                                > past_inputs.last_received_seq_num)
                            {
                                // Apply the transform update
                                list.transform_list[entity_idx] = 
                                    packet.packet.packet_data.transform_sync.transform_state;
                                past_inputs.last_received_seq_num =
                                    packet.packet.packet_data.transform_sync.sequence;

                                // apply later inputs (reconciliation)
                                for (uint32_t sequence = packet.packet.packet_data.transform_sync.sequence;
                                     sequence < past_inputs.next_seq_num;
                                     sequence++)
                                {
                                    uint32_t input_idx = sequence % ARRAY_LENGTH(past_inputs.inputs);
                                    if (past_inputs.inputs[input_idx].sequence_number == sequence)
                                    {
                                        Vec3 ship_thrust;
                                        Vec3 ship_torque;
                                        get_ship_thrust(
                                            past_inputs.inputs[input_idx].input,
                                            list.transform_list[entity_idx].orientation,
                                            &ship_thrust,
                                            &ship_torque);

                                        apply_impulse(
                                            ship_thrust * past_inputs.inputs[input_idx].dt,
                                            &list.transform_list[entity_idx].velocity,
                                            list.physics_list[entity_idx].mass);
                                        apply_angular_impulse(
                                            ship_torque * past_inputs.inputs[input_idx].dt,
                                            &list.transform_list[entity_idx].angular_velocity,
                                            list.physics_list[entity_idx].angular_mass);
                                    }
                                }
                            }
                        }
                        else    // if the update isn't for the player then always apply it
                        {
                            entity_manager->entity_lists[list_idx].transform_list[entity_idx] = 
                                packet.packet.packet_data.transform_sync.transform_state;
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
            if(entity_manager->entity_table.lookup_entity(
                   client_state.player_handle,
                   entity_manager->entity_lists,
                   &list_idx,
                   &entity_idx))
            {
                renderer.camera_pos = entity_manager->entity_lists[list_idx].transform_list[entity_idx].position;
                renderer.camera_orientation =
                    entity_manager->entity_lists[list_idx].transform_list[entity_idx].orientation;
            }
        }
        
        // =========
        // Rendering
        // =========

        renderer.clear();

        renderer.draw_skybox();

        for (unsigned int list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager->entity_lists[list_idx];
            // check the list has suitable components for rendering
            if (!entity_list.supports_components(ComponentType::TRANSFORM | ComponentType::MESH))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                MeshId mesh = entity_list.mesh_list[entity_idx];
                Transform transform = entity_list.transform_list[entity_idx];
                renderer.draw_mesh(
                    mesh,
                    transform.position,
                    transform.scale,
                    transform.orientation);
                if (!entity_list.supports_components(ComponentType::PLAYER_CONTROL))
                {
                    renderer.draw_crosshair(transform.position, 0.1f);
                }
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
                SDL_Delay((uint32_t)(1000 * floorf(time_remaining)));
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
