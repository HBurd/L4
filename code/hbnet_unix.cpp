#include "hbnet.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <cassert>
#include <vector>
#include <iostream>

ClientId ServerData::init(uint16_t port)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock >= 0);

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    assert(bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) >= 0);

    active = true;
    return SERVER_ID;
}

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

void ServerData::broadcast(GamePacket packet)
{
    for (auto client : clients)
    {
        sendto(
            sock,
            &packet,
            sizeof(packet),
            0,
            (sockaddr*)&client.addr,
            sizeof(client.addr));
    }
}

ClientId ClientData::connect(uint32_t server_ip, uint16_t server_port)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock >= 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(server_ip);
    server_addr.sin_port = htons(server_port);

    ConnectionReqPacket req_packet;

    // keep requesting until response is received
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
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);
#endif

    ConnectionAckPacket* ack_packet = (ConnectionAckPacket*)ack_packet_data;

    assert(ack_packet->header.type == GamePacketType::CONNECTION_ACK);
    assert(from.sin_addr.s_addr == server_addr.sin_addr.s_addr);
    
    client_id = ack_packet->client_id;

    active = true;
    return client_id;
}

void ClientData::send_to_server(GamePacket packet)
{
    sendto(
        sock,
        &packet,
        sizeof(packet),
        0,
        (sockaddr*)&server_addr,
        sizeof(server_addr));
}

bool recv_game_packet(int sock, GamePacket* packet, sockaddr_in* from)
{
    size_t fromlen = sizeof(*from);

    int n = recvfrom(
        sock,
        packet,
        sizeof(*packet),
        MSG_DONTWAIT,   // nonblocking
        (sockaddr*)from,
        (socklen_t*)&fromlen);

    return n != -1;     // returns false if none available (or error)
}

void ServerData::accept_client(sockaddr_in client_addr)
{
    // add to list of known clients
    ClientId client_id = clients.size();
    clients.push_back(ClientConnection(client_addr));

    // send ack
    ConnectionAckPacket ack_packet(client_id, SERVER_ID);

    sendto(
        sock,
        &ack_packet,
        sizeof(ack_packet),
        0,
        (sockaddr*)&client_addr,
        sizeof(client_addr));
}
