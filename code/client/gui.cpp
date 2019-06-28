#include "client/gui.h"
#include "common/util.h"
#include "common/components.h"
#include "common/TransformComponent.h"

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

    ImGui::InputText("IP", ip, sizeof(ip));

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
    const EntityManager *entity_manager,
    EntityRef ref,
    char* name,
    size_t name_len)
{
    const EntityListInfo &entity_list = entity_manager->entity_lists[ref.list_idx];
    if (entity_list.supports_component(ComponentType::PLAYER_CONTROL))
    {
        snprintf(
            name,
            name_len,
            "ship (id %u)",
            entity_list.handles[ref.entity_idx].idx);
    }
    else if (entity_list.supports_component(ComponentType::PROJECTILE))
    {
        snprintf(
            name,
            name_len,
            "projectile (id %u)",
            entity_list.handles[ref.entity_idx].idx);
    }
    else
    {
        snprintf(
            name,
            name_len,
            "celestial body (id %u)",
            entity_list.handles[ref.entity_idx].idx);
    }
}

Console::Console(const char *name_)
:name(name_) {}

void Console::draw()
{
    // build the string to give to imgui
    char display_data[ARRAY_LENGTH(data) + 1] = {};
    // copy mark to end
    memcpy(display_data, data + mark, ARRAY_LENGTH(data) - mark);
    // copy start to mark
    memcpy(display_data + strlen(display_data), data, mark);

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

/*    Writes string_len bytes of string to the console.
    The string will be copied in segments, so that
    no section of the string will be copied past the
    end of the console buffer. If the string is longer
    than the console buffer, later copies of the string
    will overwrite earlier copies, so as a result only
    the end of the string will be in the console buffer.
*/
void Console::writen(const char *string, size_t string_len)
{
    size_t len_copied = 0;
    while (len_copied < string_len)    // while there are still bytes in string to copy
    {
        // length to copy is min of string length and remaining length of console buffer
        size_t copy_len = (string_len - len_copied <= ARRAY_LENGTH(data) - mark)
            ? string_len - len_copied : (ARRAY_LENGTH(data) - mark);

        // copy copy_len bytes of string to start of circular buffer
        memcpy(data + mark, string + len_copied, copy_len);
        len_copied += copy_len;

        // adjust the marker of the circular buffer
        mark += copy_len;
        // wrap marker if necessary
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
    float distance;
    float relative_velocity;

    EntityRef player_ref = entity_manager->entity_table.lookup_entity(player_handle);
    EntityRef other_ref = entity_manager->entity_table.lookup_entity(other_handle);

    if (player_ref.is_valid() && other_ref.is_valid())
    {
        Transform player_transform = *(Transform*)entity_manager->lookup_component(player_ref, ComponentType::TRANSFORM);
        Transform other_transform = *(Transform*)entity_manager->lookup_component(other_ref, ComponentType::TRANSFORM);

        distance = (player_transform.position - other_transform.position).norm();
        relative_velocity = (player_transform.velocity - other_transform.velocity).norm();
    }
    else
    {
        // TODO: debug message?
        distance = 0.0f;
        relative_velocity = 0.0f;
    }

    ImGui::Text("Distance: %f", distance);
    ImGui::Text("Relative velocity: %f", relative_velocity);
}

bool draw_guidance_menu(
     const EntityManager* entity_manager,
     EntityHandle player_handle,
     TrackingState *tracking)
{
    bool entity_selected = false;
    bool entity_hovered = false;
    EntityHandle hovered_entity;

    ImGui::Begin("Guidance System", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Target tracking", &tracking->track);
    ImGui::Checkbox("Stabilization", &tracking->stabilize);

    ImGui::Text("Local objects:");
    bool something_detected = false;

    for (unsigned int list_idx = 0; list_idx < entity_manager->entity_lists.size(); list_idx++)
    {
        const EntityListInfo &entity_list = entity_manager->entity_lists[list_idx];
        if (!entity_list.supports_component(ComponentType::PHYSICS))
            continue;
        for (unsigned int entity_idx = 0; entity_idx < entity_list.size; entity_idx++)
        {
            // check if entity is player and skip
            if (entity_list.handles[entity_idx] == player_handle)
                continue;

            EntityRef ref;
            ref.list_idx = list_idx;
            ref.entity_idx = entity_idx;

            char entity_id_name[32];
            generate_entity_name(
                entity_manager,
                ref,
                entity_id_name,
                sizeof(entity_id_name));

            something_detected = true;
            if (ImGui::Button(entity_id_name))
            {
                entity_selected = true;
                tracking->guidance_target = entity_list.handles[entity_idx];
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

        EntityRef ref = entity_manager->entity_table.lookup_entity(hovered_entity);
        assert(ref.is_valid());
        
        generate_entity_name(
            entity_manager,
            ref,
            entity_id_name,
            sizeof(entity_id_name));

        ImGui::Text("Info about %s:", entity_id_name);
        draw_guidance_stats(entity_manager, player_handle, hovered_entity);
    }
    else if (tracking->guidance_target.is_valid())
    {
        EntityRef ref = entity_manager->entity_table.lookup_entity(tracking->guidance_target);

        if (ref.is_valid())
        {
            ImGui::Text("Target info:");
            draw_guidance_stats(entity_manager, player_handle, tracking->guidance_target);
        }
    }
    else
    {
        ImGui::Text("No target selected.");
    }

    ImGui::End();

    return entity_selected;
}

void entity_handle_string(EntityHandle handle, char handle_string[17])
{
    // handle is 8 bytes, 16 characters and null terminator
    snprintf(handle_string, 17, "%08x%08x", handle.version, handle.idx);
}

void draw_entity_select_menu(EntityHandle *selected_entity, const EntityManager &entity_manager)
{
    ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    EntityRef ref;
    for (ref.list_idx = 0; ref.list_idx < entity_manager.entity_lists.size(); ref.list_idx++)
    {
        for (ref.entity_idx = 0; ref.entity_idx < entity_manager.entity_lists[ref.list_idx].size; ref.entity_idx++)
        {
            EntityHandle handle = entity_manager.entity_lists[ref.list_idx].handles[ref.entity_idx];
            char handle_string[17];
            entity_handle_string(handle, handle_string);
            if (ImGui::Button(handle_string) && selected_entity)
            {
                *selected_entity = handle;
            }
        }
    }

    ImGui::End();
}
