#include "hb/packets.h"

GamePacketHeader::GamePacketHeader(GamePacketType packet_type, ClientId _sender)
: type(packet_type),
    sender(_sender)
{}

// SERVER PACKETS
// ==============

ConnectionAckPacket::ConnectionAckPacket(ClientId new_client_id)
: client_id(new_client_id)
{}

EntityCreatePacket::EntityCreatePacket(
    Entity _entity,
    EntityHandle entity_handle)
: entity(_entity),
    handle(entity_handle)
{}

PhysicsSyncPacket::PhysicsSyncPacket(EntityHandle _entity, Physics physics, uint32_t _sequence)
: entity(_entity),
    physics_state(physics),
    sequence(_sequence)
{}

PlayerDamagePacket::PlayerDamagePacket(ClientId _player)
: player(_player)
{}

// CLIENT PACKETS
// ==============

ConnectionReqPacket::ConnectionReqPacket()
{}

PlayerSpawnPacket::PlayerSpawnPacket(Vec3 _coords)
: coords(_coords)
{}

ControlUpdatePacket::ControlUpdatePacket(PlayerControlState _state, uint32_t _sequence)
: state(_state),
    sequence(_sequence)
{}

GamePacketOut::GamePacketOut(GamePacketHeader header_, void *data_, size_t data_size)
: header(header_)
{
    if (!data_)
    {
        assert(data_size == 0);
    }
    else
    {
        memcpy(data, data_, data_size);
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
