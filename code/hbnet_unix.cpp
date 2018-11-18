#include "hbnet.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <cassert>
#include <vector>

struct ClientConnection
{
    ClientConnection(sockaddr_in _addr): addr(_addr) {}
    sockaddr_in addr;
};

static void* server_thread(void* arg)
{
    ServerInfo* server_info = (ServerInfo*)arg;
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock_fd >= 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(server_info->port);
    
    assert(bind(sock_fd, (sockaddr*)&server, sizeof(server)) >= 0);

    std::vector<ClientConnection> connections;

    GamePacket recv_packet;
    sockaddr_in from;
    size_t fromlen = sizeof(from);
    while(true)
    {
        int n = recvfrom(sock_fd, &recv_packet, sizeof(recv_packet), 0, (sockaddr*)&from, (socklen_t*)&fromlen);
        assert(n == sizeof(GamePacket));
        switch (recv_packet.type)
        {
        case GamePacketType::CONNECTION_REQ:
            {
                // add to list of known clients
                connections.push_back(ClientConnection(from));
                server_info->num_clients = connections.size();

                // send ack
                GamePacket ack_packet;
                ack_packet.type = GamePacketType::CONNECTION_ACK;
                sendto(
                    sock_fd,
                    &ack_packet,
                    sizeof(ack_packet),
                    0,
                    (sockaddr*)&from,
                    sizeof(from));
            }
        }
    }

    pthread_exit(nullptr);
}

static void* client_thread(void* arg)
{
    ClientInfo* client_info = (ClientInfo*)arg;

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock_fd >= 0);
    
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(client_info->port);
    server_addr.sin_addr.s_addr = htonl(client_info->ip);

    // Connect to server
    {
        GamePacket req_packet;
        req_packet.type = GamePacketType::CONNECTION_REQ;

        sendto(
            sock_fd,
            &req_packet,
            sizeof(req_packet),
            0,
            (sockaddr*)&server_addr,
            sizeof(server_addr));

        sockaddr_in from;
        size_t fromlen = sizeof(from);
        GamePacket ack_packet;

        int n = recvfrom(
            sock_fd,
            &ack_packet,
            sizeof(ack_packet),
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);

        assert(n == sizeof(GamePacket));
        assert(ack_packet.type == GamePacketType::CONNECTION_ACK);
        assert(from.sin_addr.s_addr == server_addr.sin_addr.s_addr);

        // we successfully connected
    }

    while(true)
    {
        sockaddr_in from;
        size_t fromlen = sizeof(from);

        GamePacket packet;
        int n = recvfrom(
            sock_fd,
            &packet,
            sizeof(packet),
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);
    }

    pthread_exit(nullptr);
}

void NetworkInterface::client_connect(unsigned int ip, unsigned int port)
{
    connection_info.connection_type = ConnectionInfo::CLIENT_CONNECTION;
    connection_info.client.ip = ip;
    connection_info.client.port = port;

    pthread_t client_pthread;
    pthread_create(&client_pthread, nullptr, client_thread, &connection_info.client);
}

void NetworkInterface::server_init(unsigned int port)
{
    connection_info.connection_type = ConnectionInfo::SERVER_CONNECTION;
    connection_info.server.port = port;

    pthread_t server_pthread;
    pthread_create(&server_pthread, nullptr, server_thread, &connection_info.server);
}
