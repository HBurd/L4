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

#ifdef SERVER_GRAPHICS
// TODO: probably belongs in a different directory now that server uses this
#include "client/renderer.h"
#include "client/keyboard.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui/imgui_impl_opengl3.h"
#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600
const float PLAYER_LOOK_FACTOR = 0.005f;
#endif

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

    // Init gui if compiled with that option
#ifdef SERVER_GRAPHICS
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

    Input input;
    bool mouse_lock = false;
#endif

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
        // Need to handle window events if we have a gui
#ifdef SERVER_GRAPHICS
        input.keyboard.clear_keydowns();
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
                        input.keyboard.handle_keyup(event.key.keysym.sym);
                        break;
                    case SDL_KEYDOWN:
                        input.keyboard.handle_keydown(event.key.keysym.sym);
                        break;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Mouse wrapping
        ImGui::Begin("Debug");
        ImGui::Checkbox("Mouse lock", &mouse_lock);
        ImGui::End();
        if (mouse_lock && SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
        {
            int new_x, new_y;
            SDL_GetGlobalMouseState(&new_x, &new_y);
            int window_x, window_y;
            SDL_GetWindowPosition(window, &window_x, &window_y);
            new_x -= window_x;
            new_y -= window_y;
            input.mouse.dx = new_x - input.mouse.x;
            input.mouse.dy = new_y - input.mouse.y;
            input.mouse.x = new_x;
            input.mouse.y = new_y;

            if (input.mouse.x >= renderer.width - 1) // 1 pixel extra for when screen is max
            {
                input.mouse.x -= renderer.width - 2;
                SDL_WarpMouseInWindow(window, input.mouse.x, input.mouse.y);
            }
            else if (input.mouse.x <= 0)
            {
                input.mouse.x += renderer.width - 2;
                SDL_WarpMouseInWindow(window, input.mouse.x, input.mouse.y);
            }
            if (input.mouse.y >= renderer.height - 1)
            {
                input.mouse.y -= renderer.height - 2;
                SDL_WarpMouseInWindow(window, input.mouse.x, input.mouse.y);
            }
            else if (input.mouse.y <= 0)
            {
                input.mouse.y += renderer.height - 2;
                SDL_WarpMouseInWindow(window, input.mouse.x, input.mouse.y);
            }
        }

        Vec3 camera_velocity;
        if (input.keyboard.held.a) camera_velocity += Vec3(-1.0f, 0.0f, 0.0f);
        if (input.keyboard.held.d) camera_velocity += Vec3( 1.0f, 0.0f, 0.0f);
        if (input.keyboard.held.w) camera_velocity += Vec3( 0.0f, 0.0f,-1.0f);
        if (input.keyboard.held.s) camera_velocity += Vec3( 0.0f, 0.0f, 1.0f);
        if (input.keyboard.held.q) camera_velocity += Vec3( 0.0f,-1.0f, 0.0f); 
        if (input.keyboard.held.e) camera_velocity += Vec3( 0.0f, 1.0f, 0.0f); 

        renderer.camera_pos += TIMESTEP * camera_velocity;
        renderer.camera_orientation = renderer.camera_orientation * Rotor::yaw(PLAYER_LOOK_FACTOR * input.mouse.dx) * Rotor::pitch(PLAYER_LOOK_FACTOR * input.mouse.dy);
#endif

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

#ifdef SERVER_GRAPHICS
        renderer.clear();
        renderer.prep();
        renderer.draw_skybox();

        for (uint32_t list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
        {
            EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
            // check the list has suitable components for rendering
            if (!entity_list.supports_component(ComponentType::WORLD_SECTOR))
                continue;
            if (!entity_list.supports_component(ComponentType::TRANSFORM))
                continue;
            if (!entity_list.supports_component(ComponentType::MESH))
                continue;
            for (uint32_t entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                EntityRef ref;
                ref.list_idx = list_idx;
                ref.entity_idx = entity_idx;

                MeshId mesh = *(MeshId*)entity_manager->lookup_component(ref, ComponentType::MESH);
                WorldSector world_sector = *(WorldSector*)entity_manager->lookup_component(ref, ComponentType::WORLD_SECTOR);
                Transform transform = *(Transform*)entity_manager->lookup_component(ref, ComponentType::TRANSFORM);
                renderer.draw_mesh(
                    mesh,
                    relative_to_sector(renderer.camera_sector, world_sector, transform.position),
                    transform.scale,
                    transform.orientation);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
#endif


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
