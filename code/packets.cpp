#include "hb/packets.h"

GamePacketHeader::GamePacketHeader(int packet_type, ClientId _sender)
: type(packet_type),
    sender(_sender)
{}

// SERVER PACKETS
// ==============

ConnectionAckPacket::ConnectionAckPacket(ClientId new_client_id)
: header(GamePacketType::CONNECTION_ACK, SERVER_ID),
    client_id(new_client_id)
{}

EntityCreatePacket::EntityCreatePacket(
    Entity _entity,
    EntityHandle entity_handle)
: header(GamePacketType::ENTITY_CREATE, SERVER_ID),
    entity(_entity),
    handle(entity_handle)
{}

PhysicsSyncPacket::PhysicsSyncPacket(EntityHandle _entity, Physics physics, uint32_t _sequence)
: header(GamePacketType::PHYSICS_SYNC, SERVER_ID),
    entity(_entity),
    physics_state(physics),
    sequence(_sequence)
{}

PlayerDamagePacket::PlayerDamagePacket(ClientId _player)
: header(GamePacketType::PLAYER_DAMAGE, SERVER_ID),
    player(_player)
{}

// CLIENT PACKETS
// ==============

ConnectionReqPacket::ConnectionReqPacket()
: header(GamePacketType::CONNECTION_REQ, INCOMPLETE_ID)
{}

PlayerSpawnPacket::PlayerSpawnPacket(Vec3 _coords)
: header(GamePacketType::PLAYER_SPAWN, INCOMPLETE_ID),
    coords(_coords)
{}

ControlUpdatePacket::ControlUpdatePacket(PlayerControlState _state, uint32_t _sequence)
: header(GamePacketType::CONTROL_UPDATE, INCOMPLETE_ID),
    state(_state),
    sequence(_sequence)
{}

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
