#include "hbgui.h"
#include "hbutil.h"
#include "imgui/imgui.h"
#include <cstdio>

bool MainMenu::draw_server_create_gui()
{
    bool server_create = false;

    int port_int = (int)port;
    ImGui::InputInt("Port", &port_int);
    port = (uint16_t)(0xFFFF & port_int);

    server_create = ImGui::Button("Create Server");

    return server_create;
}

bool MainMenu::draw_server_connect_gui()
{
    bool server_connect = false;

    // InputInt4 writes to an array of ints
    // so we create an array and write it back to ip
    int ip_bytes[4] = {
        (int)(ip >> 24) & 0xFF,
        (int)(ip >> 16) & 0xFF,
        (int)(ip >> 8) & 0xFF,
        (int)ip & 0xFF};
    ImGui::InputInt4("IP", ip_bytes);
    ip = ip_bytes[0] << 24 | ip_bytes[1] << 16 | ip_bytes[2] << 8 | ip_bytes[3];

    int port_int = (int)port;
    ImGui::InputInt("Port", &port_int);
    port = (uint16_t)(0xFFFF & port_int);

    server_connect = ImGui::Button("Connect to Server");

    return server_connect;
}

void MainMenu::draw(bool* server, bool* client)
{
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Start a game or connect to a game:");

    ImGui::Checkbox("Server", &is_server);
    
    if (is_server)
    {
        *server = draw_server_create_gui();
    }
    else
    {
        *client = draw_server_connect_gui();
    }

    ImGui::End();
}

bool SpawnMenu::draw()
{
    bool spawn_clicked = false;
    ImGui::Begin("Spawn Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::Text("Choose your spawn point:");
    ImGui::InputFloat3("x, y, z", (float*)&coords);
    
    spawn_clicked = ImGui::Button("Spawn");

    ImGui::End();

    return spawn_clicked;
}

static void generate_entity_name(
    const EntityManager* entity_manager,
    size_t list_idx,
    size_t entity_idx,
    char* name,
    size_t name_len)
{
    const EntityList& entity_list = entity_manager->entity_lists[list_idx];
    if (entity_list.supports_components(ComponentType::PLAYER_CONTROL))
    {
        snprintf(
            name,
            name_len,
            "ship (id %zu)",
            entity_list.handles[entity_idx].idx);
    }
    else if (entity_list.supports_components(ComponentType::PROJECTILE))
    {
        snprintf(
            name,
            name_len,
            "projectile (id %zu)",
            entity_list.handles[entity_idx].idx);
    }
    else
    {
        snprintf(
            name,
            name_len,
            "celestial body (id %zu)",
            entity_list.handles[entity_idx].idx);
    }
}

Console::Console(const char *name_)
:name(name_) {}

void Console::draw()
{
    // build the string to give to imgui
    char display_data[ARRAY_LENGTH(data) + 1] = {};
    // copy mark to end
    strncpy(display_data, data + mark, ARRAY_LENGTH(data) - mark);
    // copy start to mark
    strncpy(display_data + strlen(display_data), data, mark);

    ImGui::Begin(name);

    ImGui::TextUnformatted(display_data);
    // set scroll to bottom
    float max_scroll = ImGui::GetScrollMaxY();
    ImGui::SetScrollY(max_scroll);

    ImGui::End();
}

void Console::write(const char *string)
{
    const size_t string_len = strlen(string);
    writen(string, string_len);
}

void Console::writen(const char *string, size_t string_len)
{
    size_t len_copied = 0;
    while (len_copied < string_len)
    {
        // copy length min of string length and remaining length of console array
        size_t copy_len = (string_len - len_copied <= ARRAY_LENGTH(data) - mark)
            ? string_len - len_copied : (ARRAY_LENGTH(data) - mark);
        strncpy(data + mark, string + len_copied, copy_len);//ARRAY_LENGTH(data) - mark);
        len_copied += copy_len;
        mark += copy_len;
        if (mark == ARRAY_LENGTH(data))
        {
            mark = 0;
        }
        assert(mark < ARRAY_LENGTH(data));
    }
}

static void draw_guidance_stats(
    const EntityManager* entity_manager,
    EntityHandle player_handle,
    EntityHandle other_handle)
{
    size_t player_list_idx;
    size_t player_entity_idx;
    size_t other_list_idx;
    size_t other_entity_idx;
    if(entity_manager->entity_table.lookup_entity(
           player_handle,
           entity_manager->entity_lists,
           &player_list_idx,
           &player_entity_idx)
       && entity_manager->entity_table.lookup_entity(
              other_handle,
              entity_manager->entity_lists,
              &other_list_idx,
              &other_entity_idx))
    {
        const EntityList& player_list = entity_manager->entity_lists[player_list_idx];
        Physics player_physics = player_list.physics_list[player_entity_idx];

        const EntityList& other_list = entity_manager->entity_lists[other_list_idx];
        Physics other_physics = other_list.physics_list[other_entity_idx];

        float distance = (player_physics.position - other_physics.position).norm();
        ImGui::Text("Distance: %f", distance);

        float relative_velocity = (player_physics.velocity - other_physics.velocity).norm();
        ImGui::Text("Relative velocity: %f", relative_velocity);
    }
}

bool draw_guidance_menu(
     const EntityManager* entity_manager,
     EntityHandle player_handle,
     EntityHandle* target_handle,
     bool* track,
     bool* stabilize)
{
    bool entity_selected = false;
    bool entity_hovered = false;
    EntityHandle hovered_entity;

    ImGui::Begin("Guidance System", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Target tracking", track);
    ImGui::Checkbox("Stabilization", stabilize);

    ImGui::Text("Local objects:");
    bool something_detected = false;

    for (unsigned int list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        const EntityList& entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_components(ComponentType::PHYSICS))
            continue;
        for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            // check if entity is player and skip
            if (entity_list.handles[entity_idx] == player_handle)
                continue;

            char entity_id_name[32];
            generate_entity_name(
                entity_manager,
                list_idx,
                entity_idx,
                entity_id_name,
                sizeof(entity_id_name));

            something_detected = true;
            if (ImGui::Button(entity_id_name))
            {
                entity_selected = true;
                *target_handle = entity_list.handles[entity_idx];
            }

            if (ImGui::IsItemHovered())
            {
                entity_hovered = true;
                hovered_entity = entity_list.handles[entity_idx];
            }
        }
    }

    if (!something_detected)
    {
        ImGui::Text("--- you are alone in deep space ---");
    }

    if (entity_hovered)
    {
        char entity_id_name[32];

        size_t list_idx;
        size_t entity_idx;
        assert(entity_manager->entity_table.lookup_entity(
            hovered_entity,
            entity_manager->entity_lists,
            &list_idx,
            &entity_idx));
        
        generate_entity_name(
            entity_manager,
            list_idx,
            entity_idx,
            entity_id_name,
            sizeof(entity_id_name));

        ImGui::Text("Info about %s:", entity_id_name);
        draw_guidance_stats(entity_manager, player_handle, hovered_entity);
    }
    else if (target_handle->is_initialized())
    {
        size_t list_idx;
        size_t entity_idx;
        if(entity_manager->entity_table.lookup_entity(
            *target_handle,
            entity_manager->entity_lists,
            &list_idx,
            &entity_idx))
        {
            ImGui::Text("Target info:");
            draw_guidance_stats(entity_manager, player_handle, *target_handle);
        }
    }
    else
    {
        ImGui::Text("No target selected.");
    }

    ImGui::End();

    return entity_selected;
}
