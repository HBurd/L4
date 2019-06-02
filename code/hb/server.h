#pragma once

#include "hb/net.h"

#include "hec.h"
#include "hb/player_control.h"

#include <vector>

struct ClientConnection
{
    ClientConnection(HbSockaddr client_addr);
    HbSockaddr addr;
    EntityHandle player_entity;
    bool received_input = false;
    uint32_t sequence;
    PlayerInputs inputs;
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
