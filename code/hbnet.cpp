#include "hbnet.h"
#include "imgui/imgui.h"

#include "hbship.h"
#include <iostream>

using std::vector;

ClientConnection::ClientConnection(sockaddr_in client_addr)
:addr(client_addr) {}

GamePacketHeader::GamePacketHeader(int packet_type, ClientId _sender)
:type(packet_type), sender(_sender) {}

ConnectionReqPacket::ConnectionReqPacket()
:header(GamePacketType::CONNECTION_REQ, INCOMPLETE_ID) {}

ConnectionAckPacket::ConnectionAckPacket(ClientId new_client_id, ClientId sender)
:header(GamePacketType::CONNECTION_ACK, sender), client_id(new_client_id) {}

PlayerSpawnPacket::PlayerSpawnPacket(Vec3 _coords, ClientId sender)
:header(GamePacketType::PLAYER_SPAWN, sender), coords(_coords) {}

EntityCreatePacket::EntityCreatePacket(
    Entity _entity,
    EntityHandle entity_handle,
    ClientId sender)
:header(GamePacketType::ENTITY_CREATE, sender), entity(_entity), handle(entity_handle) {}

ControlUpdatePacket::ControlUpdatePacket(PlayerControlState _state, ClientId sender)
:header(GamePacketType::CONTROL_UPDATE, sender), state(_state) {}

PhysicsSyncPacket::PhysicsSyncPacket(EntityHandle _entity, Physics physics, ClientId sender)
:header(GamePacketType::PHYSICS_SYNC, sender), entity(_entity), physics_state(physics) {}

bool NetworkGui::draw_server_create_gui()
{
    bool server_create = false;
    if (server_create_gui)
    {
        ImGui::Begin("Create Server", &server_create_gui);
        ImGui::InputInt("Port", (int*)&port);

        if (ImGui::Button("Create"))
        {
            // close this window
            server_create_gui = false;
            server_create = true;   // set return value
        }
        ImGui::End();
    }

    return server_create;
}

bool NetworkGui::draw_server_connect_gui()
{
    bool server_connect = false;
    if (server_connect_gui)
    {
        ImGui::Begin("Connect to Server", &server_connect_gui);

        ImGui::InputInt("IP", (int*)&ip);
        ImGui::InputInt("Port", (int*)&port);

        if (ImGui::Button("Connect"))
        {
            // close this window
            server_connect_gui = false;
            server_connect = true;  // set return value
        }
        ImGui::End();
    }
    return server_connect;
}

void NetworkGui::draw_main_gui()
{
    ImGui::Begin("Network", &main_gui);

    if (ImGui::Button("Create Server"))
    {
        server_create_gui = true;
    }
    if (ImGui::Button("Connect to server"))
    {
        server_connect_gui = true;
    }
    //if (connection_info.connection_type == ConnectionInfo::SERVER_CONNECTION)
    //{
    //    // Technically we should lock here, but really there's no problem
    //    // with reading a garbage value
    //    ImGui::Text("Server running. %zu clients connected.",
    //                connection_info.server->connections.size());
    //}
    //else if (connection_info.connection_type == ConnectionInfo::CLIENT_CONNECTION)
    //{
    //    ImGui::Text("Client connected with client id %zu.",
    //                connection_info.client->client_id);
    //}
    ImGui::End();
}

void NetworkGui::draw(bool* server, bool* client)
{
    if (main_gui)
    {
        draw_main_gui();
        *server = draw_server_create_gui();
        *client = draw_server_connect_gui();
    }
}

void get_packets(int sock, vector<GamePacketIn>* packet_list)
{
    packet_list->clear();

    uint8_t packet[sizeof(GamePacket)];
    sockaddr_in from;
    while (recv_game_packet(sock, (GamePacket*)&packet, &from))
    {
        GamePacket* game_packet = (GamePacket*) packet;
        packet_list->push_back({from, *game_packet});
    }
}

void ClientData::spawn(Vec3 coords)
{
    PlayerSpawnPacket spawn_packet(coords, client_id);;
    send_to_server(*(GamePacket*)&spawn_packet);
}
