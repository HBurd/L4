#include "hbnet.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
#include <vector>
#include <iostream>

#if 0
static void* server_thread(void* arg)
{
    ConnectionInfo* connection_info = (ConnectionInfo*)arg;
    connection_info->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(connection_info->sock_fd >= 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(connection_info->server->port);
    
    assert(bind(connection_info->sock_fd, (sockaddr*)&server, sizeof(server)) >= 0);

    while(true)
    {
        uint8_t recv_packet[sizeof(GamePacket)] = {};
        sockaddr_in from;
        size_t fromlen = sizeof(from);

        recvfrom(
            connection_info->sock_fd,
            recv_packet,
            sizeof(recv_packet),
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);
        
        assert(fromlen == sizeof(from));

        GamePacketHeader* recv_packet_header = (GamePacketHeader*)recv_packet;
        switch (recv_packet_header->type)
        {
            case GamePacketType::CONNECTION_REQ:
            {
                // add to list of known clients
                ClientId client_id;
                {
                    client_id = connection_info->server->connections.size();
                    std::lock_guard<std::mutex> lock(connection_info->server->mutex);
                    connection_info->server->connections.push_back(from);
                }

                // send ack
                ConnectionAckPacket ack_packet(client_id, SERVER_ID);

                sendto(
                    connection_info->sock_fd,
                    &ack_packet,
                    sizeof(ack_packet),
                    0,
                    (sockaddr*)&from,
                    sizeof(from));
            } break;
            default:
            // if not handled above, any incoming packet goes directly into the command queue
            {
                std::lock_guard<std::mutex> lock(connection_info->mutex);
                connection_info->back_queue->push_back(*(GamePacket*)recv_packet);
            }
        }
    }

    pthread_exit(nullptr);
}

static void* client_thread(void* arg)
{
    ConnectionInfo* connection_info = (ConnectionInfo*)arg;

    connection_info->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(connection_info->sock_fd >= 0);
    
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(connection_info->client->port);
    server_addr.sin_addr.s_addr = htonl(connection_info->client->ip);

    // Connect to server
    {
        ConnectionReqPacket req_packet;

        sendto(
            connection_info->sock_fd,
            &req_packet,
            sizeof(req_packet),
            0,
            (sockaddr*)&server_addr,
            sizeof(server_addr));

        sockaddr_in from = {};
        size_t fromlen = sizeof(from);
        
        uint8_t ack_packet_data[sizeof(GamePacket)] = {};

        int n = recvfrom(
            connection_info->sock_fd,
            ack_packet_data,
            sizeof(ack_packet_data),
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);

        ConnectionAckPacket* ack_packet = (ConnectionAckPacket*)ack_packet_data;

        assert(ack_packet->header.type == GamePacketType::CONNECTION_ACK);
        assert(from.sin_addr.s_addr == server_addr.sin_addr.s_addr);
        
        // TODO: Lock here?
            
        connection_info->client->client_id = ack_packet->client_id;

        // we successfully connected
        // now request a spawn
        PlayerSpawnPacket spawn_packet(Vec3(0.0f, 0.0f, -3.0f), connection_info->client->client_id);

        sendto(
            connection_info->sock_fd,
            &spawn_packet,
            sizeof(spawn_packet),
            0,
            (sockaddr*)&server_addr,
            sizeof(server_addr));
    }

    while(true)
    {
        uint8_t recv_packet[sizeof(GamePacket)] = {};
        sockaddr_in from;
        size_t fromlen = sizeof(from);

        recvfrom(
            connection_info->sock_fd,
            recv_packet,
            sizeof(recv_packet),
            0,
            (sockaddr*)&from,
            (socklen_t*)&fromlen);
        
        assert(fromlen == sizeof(from));

        GamePacketHeader* recv_packet_header = (GamePacketHeader*)recv_packet;

        // stick the recieved packet in the command queue
        std::lock_guard<std::mutex> lock(connection_info->mutex);
        connection_info->back_queue->push_back(*(GamePacket*)recv_packet);
    }

    pthread_exit(nullptr);
}

void PacketReceiver::thread_init(uint16_t _port)
{
    port = _port;

    // TODO: Store the thread
    pthread_t server_pthread;
    pthread_create(&server_pthread, nullptr, receiver_thread, this);
}

void NetworkServerInstance::init(uint16_t _port)
{
    port = _port;
    receiver.thread_init(_port);
    active = true;
}

void NetworkClientInstance::client_connect()
{
    ConnectionReqPacket req_packet;

    sendto(
        connection_info->sock_fd,
        &req_packet,
        sizeof(req_packet),
        0,
        (sockaddr*)&server_addr,
        sizeof(server_addr));

    sockaddr_in from = {};
    size_t fromlen = sizeof(from);
    
    uint8_t ack_packet_data[sizeof(GamePacket)] = {};

    int n = recvfrom(
        sock_fd,
        ack_packet_data,
        sizeof(ack_packet_data),
        0,
        (sockaddr*)&from,
        (socklen_t*)&fromlen);

    ConnectionAckPacket* ack_packet = (ConnectionAckPacket*)ack_packet_data;

    assert(ack_packet->header.type == GamePacketType::CONNECTION_ACK);
    assert(from.sin_addr.s_addr == server_addr.sin_addr.s_addr);
    
    // TODO: Lock here?
        
    connection_info->client->client_id = ack_packet->client_id;
}

void NetworkClientInstance::client(uint32_t ip, uint16_t port)
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock_fd >= 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(ip);

    client_connect();

    active = true;

    pthread_t client_pthread;
    pthread_create(&client_pthread, nullptr, client_thread, &connection_info);
}

void NetworkInterface::broadcast_create_entity(Entity entity, EntityHandle entity_handle)
{
    EntityCreatePacket packet(entity, entity_handle, SERVER_ID);
    
    std::lock_guard<std::mutex> lock(connection_info.server->mutex);
    for (auto client : connection_info.server->connections)
    {
        sendto(
            connection_info.sock_fd,
            &packet,
            sizeof(packet),
            0,
            (sockaddr*)&client.addr,
            sizeof(client.addr));
    }
}
#endif

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

    sendto(
        sock,
        &req_packet,
        sizeof(req_packet),
        0,
        (sockaddr*)&server_addr,
        sizeof(server_addr));

    sockaddr_in from = {};
    size_t fromlen = sizeof(from);
    
    uint8_t ack_packet_data[sizeof(GamePacket)] = {};

    int n = recvfrom(
        sock,
        ack_packet_data,
        sizeof(ack_packet_data),
        0,
        (sockaddr*)&from,
        (socklen_t*)&fromlen);

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
