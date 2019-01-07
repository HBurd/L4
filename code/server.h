#include "hb/server.h"

ClientConnection::ClientConnection(sockaddr_in client_addr)
:addr(client_addr) {}

ServerData::ServerData(uint16_t port)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock >= 0);

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    assert(bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) >= 0);
}

void ServerData::broadcast(GamePacket &packet)
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

void ServerData::accept_client(sockaddr_in client_addr)
{
    // add to list of known clients
    ClientId client_id = clients.size();
    clients.push_back(ClientConnection(client_addr));

    // send ack
    ConnectionAckPacket ack_packet(client_id);

    sendto(
        sock,
        &ack_packet,
        sizeof(ack_packet),
        0,
        (sockaddr*)&client_addr,
        sizeof(client_addr));
}
