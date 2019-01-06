#ifndef HBSERVER_H
#define HBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "hbplayer_control.h"
#include "hbentities.h"

union GamePacket;

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
    void broadcast(GamePacket &packet);
    void accept_client(sockaddr_in client_addr);
};

#endif // include guard
