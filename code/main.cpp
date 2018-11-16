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

    EntityManager entity_manager;
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH));
    entity_manager.entity_lists.push_back(
        EntityList(ComponentType::PHYSICS | ComponentType::MESH | ComponentType::PLAYER_CONTROL));

    // add test entities
    EntityHandle test_entity1;
    {
        Entity player_ship_entity = create_ship(Vec3(-1.5f, 0.0f, -3.0f), ship_mesh);
        player_ship_entity.supported_components |= ComponentType::PLAYER_CONTROL;
        EntityHandle test_entity1 = entity_manager.create_entity(player_ship_entity);
        EntityHandle test_entity2 = entity_manager.create_entity(
            create_ship(Vec3(1.5f, 0.0f, -3.0f), ship_mesh));
        EntityHandle test_entity3 = entity_manager.create_entity(
            create_ship(Vec3(0.0f, 0.0f, 2.0f), ship_mesh));
    }
 
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
                //Rotor omega = Rotor::yaw(0.003f);
                //entity_list.physics_list[entity_idx].orientation = 
                //    omega * entity_list.physics_list[entity_idx].orientation;
            }
        }

        // PlayerControl updates
        for (unsigned int list_idx = 0; list_idx < entity_manager.entity_lists.size(); list_idx++)
        {
            EntityList& entity_list = entity_manager.entity_lists[list_idx];
            if (!entity_list.supports_components(ComponentType::PHYSICS | ComponentType::PLAYER_CONTROL))
                continue;
            for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
            {
                player_control_update(&entity_list.physics_list[entity_idx], kb);
            }
        }

        // Camera update
        {
            size_t list_idx;
            size_t entity_idx;

            entity_manager.entity_table.lookup_entity(
                test_entity1,
                entity_manager.entity_lists,
                &list_idx,
                &entity_idx);

            Physics& physics = entity_manager.entity_lists[list_idx].physics_list[entity_idx];
            renderer.camera_pos = physics.position;
            renderer.camera_orientation = physics.orientation;
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
