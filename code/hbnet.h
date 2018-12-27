#ifndef HBNET_H
#define HBNET_H

#include <cstdlib>
#include <stdint.h>

// TODO try to get rid of these!!
#include <sys/socket.h>
#include <netinet/in.h>

#include "hbentities.h"
#include "hbrenderer.h"
#include "hbplayer_control.h"

typedef size_t ClientId;
const size_t SERVER_ID = 0xFFFFFFFF;
const size_t INCOMPLETE_ID = 0xFFFFFFFE;

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
    
    ConnectionAckPacket(ClientId new_client_id, ClientId sender);
};

struct EntityCreatePacket
{
    GamePacketHeader header;
    Entity entity;
    EntityHandle handle;

    EntityCreatePacket(Entity _entity, EntityHandle entity_handle, ClientId sender);
};

struct PlayerSpawnPacket
{
    GamePacketHeader header;
    Vec3 coords;

    PlayerSpawnPacket(Vec3 _coords, ClientId sender);
};

struct ControlUpdatePacket
{
    GamePacketHeader header;
    PlayerControlState state;
    
    ControlUpdatePacket(PlayerControlState _state, ClientId sender);
};

struct PhysicsSyncPacket
{
    GamePacketHeader header;
    EntityHandle entity;
    Physics physics_state;

    PhysicsSyncPacket(EntityHandle _entity, Physics physics, ClientId sender);
};

struct PlayerDamagePacket
{
    GamePacketHeader header;
    ClientId player;
    // implicitly player entity for now
    
    PlayerDamagePacket(ClientId _player, ClientId sender);
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

struct ClientConnection
{
    ClientConnection(sockaddr_in client_addr);
    sockaddr_in addr;
    EntityHandle player_entity;
    PlayerControlState player_control;
};

struct ServerData
{
    bool active = false;
    std::vector<ClientConnection> clients;
    int sock = -1;

    ClientId init(uint16_t port);
    void broadcast(GamePacket packet);
    void accept_client(sockaddr_in client_addr);
};

struct ClientData
{
    bool active = false;
    std::vector<ClientConnection> clients;
    int sock = -1;
    sockaddr_in server_addr = {};
    ClientId client_id;

    ClientId connect(uint32_t server_ip, uint16_t server_port);
    void send_to_server(GamePacket packet);
    void spawn(Vec3 coords);
};

void get_packets(int sock, vector<GamePacketIn>* packet_list);
bool recv_game_packet(int sock, GamePacket* packet, sockaddr_in* from);

#endif // include guard
