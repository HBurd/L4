#ifndef HBCLIENT_H
#define HBCLIENT_H

#include "hbnet.h"

//TODO: shouldn't have this dependency!!
#include "hbgui.h"

struct ClientData
{
    bool active = false;
    int sock = -1;
    sockaddr_in server_addr = {};
    ClientId client_id;

    ClientId id = INCOMPLETE_ID;

    int server_pipe = -1;       // TODO: platform specific
    // write stdout of the server process to a circular buffer
    void write_server_stdout(Console *console);

    void connect(uint32_t server_ip, uint16_t server_port);
    void send_to_server(GamePacket &packet);
    void spawn(Vec3 coords);
    void create_server(uint16_t port);
};

#endif // include guard
