#include "hbnet.h"
#include "imgui/imgui.h"

void NetworkInterface::draw_server_create_gui()
{
    if (gui_state.draw_server_create_gui)
    {
        ImGui::Begin("Create Server", &gui_state.draw_server_create_gui);
        ImGui::InputInt("Port", (int*)&gui_state.port);

        if (ImGui::Button("Create"))
        {
            server_init(gui_state.port);
            gui_state.draw_server_create_gui = false;
        }
        ImGui::End();
    }
}

void NetworkInterface::draw_server_connect_gui()
{
    if (gui_state.draw_server_connect_gui)
    {
        ImGui::Begin("Connect to Server", &gui_state.draw_server_connect_gui);

        ImGui::InputInt("IP", (int*)&gui_state.ip, ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputInt("Port", (int*)&gui_state.port);

        if (ImGui::Button("Connect"))
        {
            client_connect(gui_state.ip, gui_state.port);
            gui_state.draw_server_connect_gui = false;
        }
        ImGui::End();
    }
}

void NetworkInterface::draw_main_gui()
{
    if (gui_state.draw_main_gui)
    {
        ImGui::Begin("Network", &gui_state.draw_main_gui);

        if (ImGui::Button("Create Server"))
        {
            gui_state.draw_server_create_gui = true;
        }
        if (ImGui::Button("Connect to server"))
        {
            gui_state.draw_server_connect_gui = true;
        }
        if (connection_info.connection_type == ConnectionInfo::SERVER_CONNECTION)
        {
            ImGui::Text("Server running. %zu clients connected.",
                        (size_t)connection_info.server.num_clients);
        }
        ImGui::End();
    }
}

void NetworkInterface::draw_gui()
{
    draw_main_gui();
    draw_server_create_gui();
    draw_server_connect_gui();
}
