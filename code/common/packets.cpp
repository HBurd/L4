#include "hb/packets.h"
#include <cassert>

using std::vector;

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
    EntityHandle entity_handle)
:
    handle(entity_handle)
    // caller has to set the entity data
{}

TransformSyncPacket::TransformSyncPacket(
    EntityHandle transform_entity,
    Transform transform,
    uint32_t packet_sequence)
: entity(transform_entity),
    transform_state(transform),
    sequence(packet_sequence)
{}

// CLIENT PACKETS
// ==============

ConnectionReqPacket::ConnectionReqPacket()
{}

PlayerSpawnPacket::PlayerSpawnPacket(Vec3 _coords)
: coords(_coords)
{}

ControlUpdatePacket::ControlUpdatePacket(PlayerInputs player_inputs, uint32_t input_sequence)
    : inputs(player_inputs),
      sequence(input_sequence)
{}

GamePacketOut::GamePacketOut(GamePacketHeader header_)
: header(header_)
{
}

void get_packets(HbSocket sock, vector<GamePacketIn>* packet_list)
{
    packet_list->clear();

    uint8_t packet[sizeof(GamePacket)];
    HbSockaddr from;
    while (recv_game_packet(sock, (GamePacket*)&packet, &from))
    {
        GamePacket* game_packet = (GamePacket*) packet;
        packet_list->push_back({from, *game_packet});
    }

}

size_t make_entity_create_packet(EntityRef ref, EntityManager *entity_manager, uint8_t *data, size_t size)
{
    // TODO this is tricky
    EntityCreatePacket *create_packet = new (data) EntityCreatePacket(entity_manager->entity_lists[ref.list_idx].handles[ref.entity_idx]);

    assert(size > sizeof(EntityCreatePacket));
    create_packet->handle = entity_manager->entity_lists[ref.list_idx].handles[ref.entity_idx];
    create_packet->data_size = size - sizeof(EntityCreatePacket);
    assert(create_packet->data_size >= entity_manager->serialize_entity_size(ref));
    entity_manager->serialize_entity(ref, create_packet->entity_data, &create_packet->data_size);

    return sizeof(EntityCreatePacket) + create_packet->data_size;
}
