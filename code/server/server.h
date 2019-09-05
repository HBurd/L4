#pragma once

#include "common/net.h"

#include "hec.h"
#include "client/player_input.h"

#include <vector>

struct ClientConnection
{
    ClientConnection(HbSockaddr client_addr);
    HbSockaddr addr;
    EntityHandle player_entity;
    bool received_input = false;
    uint32_t sequence = 0;
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
