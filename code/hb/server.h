#ifndef HBSERVER_H
#define HBSERVER_H

#include "hb/net.h"

#include "hb/player_control.h"
#include "hec.h"

struct ClientConnection
{
    ClientConnection(HbSockaddr client_addr);
    HbSockaddr addr;
    EntityHandle player_entity;
    bool received_input = false;
    uint32_t sequence;
    PlayerControlState player_control;
};

struct ServerData
{
    std::vector<ClientConnection> clients;
    HbSocket sock;

    ServerData(uint16_t port);
    void broadcast(
        GamePacketType packet_type,
        void *packet,
        size_t packet_size);
    ClientId accept_client(HbSockaddr client_addr);
};

#ifdef FAST_BUILD
#include "server.cpp"
#endif

#endif // include guard
