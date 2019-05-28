#ifndef HBPACKETS_H
#define HBPACKETS_H

#include <vector>

#include "hb/net.h"
#include "hec.h"
#include "hb/math.h"
#include "hb/player_control.h"
#include "hb/renderer.h"
#include "hb/TransformComponent.h"

struct GamePacketHeader
{
    GamePacketType type;
    ClientId sender;

    GamePacketHeader(GamePacketType packet_type, ClientId _sender);
};

struct ConnectionReqPacket
{
    ConnectionReqPacket();
};

struct ConnectionAckPacket
{
    ClientId client_id;
    
    ConnectionAckPacket(ClientId new_client_id);
};

struct EntityCreatePacket
{
    EntityHandle handle;
    size_t data_size;
    uint8_t entity_data[];

    EntityCreatePacket(EntityHandle entity_handle);
};

size_t make_entity_create_packet(EntityRef ref, EntityManager *entity_manager, uint8_t *data, size_t size);

struct PlayerSpawnPacket
{
    Vec3 coords;

    PlayerSpawnPacket(Vec3 _coords);
};

struct ControlUpdatePacket
{
    PlayerControlState state;
    uint32_t sequence;
    
    ControlUpdatePacket(PlayerControlState _state, uint32_t _sequence);
};

struct TransformSyncPacket
{
    EntityHandle entity;
    Transform transform_state;
    uint32_t sequence;

    TransformSyncPacket(
        EntityHandle transform_entity,
        Transform transform,
        uint32_t packet_sequence);
};

struct PlayerDamagePacket
{
    ClientId player;
    // implicitly player entity for now
    
    PlayerDamagePacket(ClientId _player);
};

// ===============================
// ^^ ADD NEW PACKET TYPES HERE ^^
// ===============================

// Basically some sort of franken-type which can't be directly instantiated
// but can be casted to. Useful occasionally for storing lists of packets
struct GamePacket
{
    GamePacketHeader header;
    union GamePacketData
    {
        ConnectionReqPacket connection_req;
        ConnectionAckPacket connection_ack;
        PlayerSpawnPacket player_spawn;
        EntityCreatePacket entity_create;
        ControlUpdatePacket control_update;
        TransformSyncPacket transform_sync;
        PlayerDamagePacket player_damage;
        // ===============================
        // ^^ ADD NEW PACKET TYPES HERE ^^
        // ===============================

        // TODO: Temporary hack for safely handling variable length packets
        uint8_t buffer[1500];
    } packet_data;
};

struct GamePacketOut
{
    GamePacketHeader header;
    uint8_t data[];

    // data can be null if size is 0
    GamePacketOut(GamePacketHeader header_);
};

struct GamePacketIn
{
    HbSockaddr sender;
    GamePacket packet;
};

void get_packets(HbSocket sock, std::vector<GamePacketIn>* packet_list);

#endif
