#include "hbnet.h"
#include "hbship.h"

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
