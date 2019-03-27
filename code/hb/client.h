#ifndef HBCLIENT_H
#define HBCLIENT_H

#include "hb/net.h"

//TODO: shouldn't have this dependency!!
#include "hb/gui.h"

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE HbPipe;
#endif

#ifdef __unix__
typedef int HbPipe;
#endif

struct ClientData
{
    bool active = false;
    HbSocket sock = HB_INVALID_SOCKET;
    HbSockaddr server_addr = {};
    ClientId client_id;

    ClientId id = INCOMPLETE_ID;

    HbPipe server_pipe;

    // write stdout of the server process to a circular buffer
    void write_server_stdout(Console *console);

    bool connect(uint32_t server_ip, uint16_t server_port);
    void send_to_server(GamePacketType type, void *packet_data, size_t data_size);
    void spawn(Vec3 coords);
    void create_server(uint16_t port);
};

#ifdef FAST_BUILD
#include "client.cpp"
#endif

#endif // include guard
