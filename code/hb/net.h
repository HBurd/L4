#ifndef HBNET_H
#define HBNET_H

#include <cstdlib>
#include <stdint.h>

#ifdef __unix__
	#include <netinet/in.h>

	typedef int HbSocket;
	const int HB_INVALID_SOCKET = -1;
	typedef sockaddr_in HbSockaddr;
#endif
#ifdef _WIN32
	#include <Winsock2.h>

	typedef SOCKET HbSocket;
	const SOCKET HB_INVALID_SOCKET = INVALID_SOCKET;
	typedef sockaddr_in HbSockaddr;
#endif

// host byte ordering
HbSockaddr create_sockaddr(uint32_t ip, uint16_t port);

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

HbSocket create_game_socket();

void send_game_packet(
    HbSocket sock,
    HbSockaddr to,
    ClientId sender_id,
    GamePacketType packet_type,
    void *packet_data,
    size_t data_size);
bool recv_game_packet(
    HbSocket sock,
    GamePacket* packet,
    HbSockaddr* from);

#ifdef FAST_BUILD
#include "net.cpp"
#endif

#endif // include guard
