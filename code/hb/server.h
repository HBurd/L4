#ifndef HBSERVER_H
#define HBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "hb/player_control.h"
#include "hb/entities.h"

struct ClientConnection
{
    ClientConnection(sockaddr_in client_addr);
    sockaddr_in addr;
    EntityHandle player_entity;
    bool received_input = false;
    uint32_t sequence;
    PlayerControlState player_control;
};

struct ServerData
{
    std::vector<ClientConnection> clients;
    int sock;

    ServerData(uint16_t port);
    void broadcast(
        GamePacketType packet_type,
        void *packet,
        size_t packet_size);
    ClientId accept_client(sockaddr_in client_addr);
};

#ifdef FAST_BUILD
#include "server.cpp"
#endif

#endif // include guard
