#include "hb/server.h"

ClientConnection::ClientConnection(HbSockaddr client_addr)
:addr(client_addr) {}

ServerData::ServerData(uint16_t port)
{
	sock = create_game_socket();
	bind_game_socket(sock, port);
}

void ServerData::broadcast(GamePacketType type, void *packet_data, size_t data_size)
{
    for (auto client : clients)
    {
        send_game_packet(
            sock,
            client.addr,
            SERVER_ID,
            type,
            packet_data,
            data_size);
    }
}

ClientId ServerData::accept_client(HbSockaddr client_addr)
{
    // add to list of known clients
    ClientId client_id = clients.size();
    clients.push_back(ClientConnection(client_addr));

    // send ack
    ConnectionAckPacket ack_packet(client_id);
    send_game_packet(
        sock,
        client_addr,
        SERVER_ID,
        GamePacketType::CONNECTION_ACK,
        &ack_packet,
        sizeof(ack_packet));

    return client_id;
}
