#ifndef HBNET_H
#define HBNET_H

#include <cstdlib>
#include <mutex>
#include <atomic>
#include <stdint.h>

#include "hbentities.h"

const unsigned int SERVER_PORT = 4444;

namespace GamePacketType
{
    enum GamePacketType
    {
        CONNECTION_REQ,    // client --> server
        CONNECTION_ACK,    // server --> client
        ENTITY_CREATE,     // server --> client
    };
}

typedef size_t ClientId;

struct __attribute__((__packed__)) GamePacket
{
    int type;

    union GamePacketData
    {
        struct ConnectionAck
        {
            ClientId client_id;
        } connection_ack;

        struct EntityCreate
        {
            EntityHandle handle;
        } entity_create;
    } packet_data;
};

struct ServerInfo
{
    std::atomic<size_t> num_clients{0};
    uint32_t port;
};

struct ClientInfo
{
    uint32_t ip;
    uint16_t port;
};

struct ConnectionInfo
{
    enum ConnectionType
    {
        NO_CONNECTION,
        SERVER_CONNECTION,
        CLIENT_CONNECTION
    } connection_type = NO_CONNECTION;

    // only one of these is used based on the connection type
    ServerInfo server;
    ClientInfo client;

    std::vector<GamePacket> command_queue;
};

struct NetworkInterface
{
    struct GuiState
    {
        bool draw_main_gui = true;
        bool draw_server_create_gui = false;
        bool draw_server_connect_gui = false;

        uint16_t port = SERVER_PORT;
        uint32_t ip = 0x7F000001;  // 127.0.0.1
    } gui_state;
    
    ConnectionInfo connection_info;

    void draw_gui();
    void draw_main_gui();
    void draw_server_create_gui();
    void draw_server_connect_gui();

    void server_init(unsigned int port);
    void client_connect(unsigned int ip, unsigned int port);
};

#endif // include guard
