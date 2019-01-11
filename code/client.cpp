#include "hb/client.h"
#include "hb/packets.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <iostream>

void ClientData::create_server(uint16_t port)
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        std::cout << "error creating pipe" << std::endl;
    }
    // create the server process
    if (fork() == 0)
    {
        // close the read end of the pipe and remap stdout
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        // we are child process
        char port_arg[6];   // 2^16 has max 5 digits, plus null terminator
        snprintf(port_arg, sizeof(port_arg), "%d", port);
        // replace process
        execl("L4server", "L4server", port_arg);
    }
    else
    {
        close(pipefd[1]);
        server_pipe = pipefd[0];
        fcntl(server_pipe, F_SETFL, O_NONBLOCK);
    }
}

void ClientData::write_server_stdout(Console *console)
{
    char buf[256];
    while(true)
    {
        int n = read(server_pipe, buf, sizeof(buf));
        if (n == -1)
        {
            if (errno != EAGAIN)
            {
                std::cout << "error reading from pipe" << std::endl;
            }
            break;
        }
        if (n == 0)
        {
            break;
        }
        console->writen(buf, n);
    }
}

void ClientData::connect(uint32_t server_ip, uint16_t server_port)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock >= 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(server_ip);
    server_addr.sin_port = htons(server_port);

    sockaddr_in from = {};
    size_t fromlen = sizeof(from);
    uint8_t ack_packet_data[sizeof(GamePacket)] = {};

    // TODO: we may need to retry several times if the
    // server hasn't started yet (i.e. client creating
    // server and connecting right away). This implementation
    // doesn't work because an extra request gets sent
    // and the server has two connections to the client.
    // Maybe the connection requires another step?
    // -- dec 27 '18
#if 0
    bool ack_received = false;
    while (!ack_received)
    {
        sendto(
            sock,
            &req_packet,
            sizeof(req_packet),
            0,
            (sockaddr*)&server_addr,
            sizeof(server_addr));


        int n = recvfrom(
            sock,
            ack_packet_data,
            sizeof(ack_packet_data),
            MSG_DONTWAIT,   // nonblocking
            (sockaddr*)&from,
            (socklen_t*)&fromlen);

        if (n >= 0) ack_received = true;
        else usleep(100000);
    }
#else
    send_game_packet(
        sock,
        server_addr,
        INCOMPLETE_ID,
        GamePacketType::CONNECTION_REQ,
        nullptr,
        0);

    int n = recvfrom(
        sock,
        ack_packet_data,
        sizeof(ack_packet_data),
        0,
        (sockaddr*)&from,
        (socklen_t*)&fromlen);
#endif

    GamePacket* ack_packet = (GamePacket*)ack_packet_data;

    assert(ack_packet->header.type == GamePacketType::CONNECTION_ACK);
    assert(from.sin_addr.s_addr == server_addr.sin_addr.s_addr);
    
    id = ack_packet->packet_data.connection_ack.client_id;

    active = true;
}

void ClientData::send_to_server(GamePacketType type, void *packet_data, size_t data_size)
{
    send_game_packet(sock, server_addr, id, type, packet_data, data_size);
}

void ClientData::spawn(Vec3 coords)
{
    PlayerSpawnPacket spawn_packet(coords);
    send_to_server(GamePacketType::PLAYER_SPAWN, &spawn_packet, sizeof(spawn_packet));
}
