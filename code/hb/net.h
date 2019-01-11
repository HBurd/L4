#ifndef HBNET_H
#define HBNET_H

#include <cstdlib>
#include <stdint.h>

// TODO try to get rid of these!!
#include <sys/socket.h>
#include <netinet/in.h>

typedef size_t ClientId;
const size_t SERVER_ID = 0xFFFFFFFF;
const size_t INCOMPLETE_ID = 0xFFFFFFFE;

// packets are declared in hbpackets.h
struct GamePacket;

enum class GamePacketType : uint8_t
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

void send_game_packet(
    int sock,
    sockaddr_in to,
    ClientId sender_id,
    GamePacketType packet_type,
    void *packet_data,
    size_t data_size);
bool recv_game_packet(
    int sock,
    GamePacket* packet,
    sockaddr_in* from);

#ifdef FAST_BUILD
#include "net.cpp"
#endif

#endif // include guard
