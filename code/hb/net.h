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
union GamePacket;

bool recv_game_packet(int sock, GamePacket* packet, sockaddr_in* from);

#ifdef FAST_BUILD
#include "net.cpp"
#endif

#endif // include guard
