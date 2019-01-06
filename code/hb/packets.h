#ifndef HBPACKETS_H
#define HBPACKETS_H

#include "hb/net.h"
#include "hb/entities.h"
#include "hb/math.h"
#include "hb/player_control.h"
#include "hb/renderer.h"

namespace GamePacketType
{
    enum GamePacketType
    {
        CONNECTION_REQ,    // client --> server
        CONNECTION_ACK,    // server --> client
        ENTITY_CREATE,     // server --> client
        PLAYER_SPAWN,      // client --> server
        CONTROL_UPDATE,    // client --> server
        PHYSICS_SYNC,      // server --> client
        PLAYER_DAMAGE,     // server --> client

        // ===============================
        // ^^ ADD NEW PACKET TYPES HERE ^^
        // ===============================
    };
}

struct GamePacketHeader
{
    int type;
    ClientId sender;

    GamePacketHeader(int packet_type, ClientId _sender);
};

struct ConnectionReqPacket
{
    GamePacketHeader header;

    ConnectionReqPacket();
};

struct ConnectionAckPacket
{
    GamePacketHeader header;
    ClientId client_id;
    
    ConnectionAckPacket(ClientId new_client_id);
};

struct EntityCreatePacket
{
    GamePacketHeader header;
    Entity entity;
    EntityHandle handle;

    EntityCreatePacket(Entity _entity, EntityHandle entity_handle);
};

struct PlayerSpawnPacket
{
    GamePacketHeader header;
    Vec3 coords;

    PlayerSpawnPacket(Vec3 _coords);
};

struct ControlUpdatePacket
{
    GamePacketHeader header;
    PlayerControlState state;
    uint32_t sequence;
    
    ControlUpdatePacket(PlayerControlState _state, uint32_t _sequence);
};

struct PhysicsSyncPacket
{
    GamePacketHeader header;
    EntityHandle entity;
    Physics physics_state;
    uint32_t sequence;

    PhysicsSyncPacket(EntityHandle _entity, Physics physics, uint32_t _sequence);
};

struct PlayerDamagePacket
{
    GamePacketHeader header;
    ClientId player;
    // implicitly player entity for now
    
    PlayerDamagePacket(ClientId _player);
};

// ===============================
// ^^ ADD NEW PACKET TYPES HERE ^^
// ===============================

// Basically some sort of franken-type which can't be directly instantiated
// but can be casted to. Useful occasionally for storing lists of packets
union GamePacket
{
    GamePacketHeader header;
    ConnectionReqPacket connection_req;
    ConnectionAckPacket conncetion_ack;
    PlayerSpawnPacket player_spawn;
    EntityCreatePacket entity_create;
    ControlUpdatePacket control_update;
    PhysicsSyncPacket physics_sync;
    PlayerDamagePacket player_damage;

    // ===============================
    // ^^ ADD NEW PACKET TYPES HERE ^^
    // ===============================
};

struct GamePacketIn
{
    sockaddr_in sender;
    GamePacket packet;
};

void get_packets(int sock, vector<GamePacketIn>* packet_list);

#endif
