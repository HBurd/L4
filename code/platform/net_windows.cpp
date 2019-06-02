#include <Winsock2.h>
#include <cassert>

#include "common/net.h"

HbSocket create_game_socket()
{
    // Initialize Winsock2
    WSADATA wsadata;
    assert(WSAStartup(MAKEWORD(2, 2), &wsadata) == 0);

    // Create the socket
    HbSocket sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != INVALID_SOCKET);

    // Set it to nonblocking
    u_long nonblocking = 1;
    ioctlsocket(sock, FIONBIO, &nonblocking);

    return sock;
}

void bind_game_socket(HbSocket sock, uint16_t port)
{
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    assert(bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) != SOCKET_ERROR);
}

void send_game_packet(
    HbSocket sock,
    HbSockaddr to,
    ClientId sender_id,
    GamePacketType packet_type,
    void *packet_data,
    size_t data_size)
{
    uint8_t *out_packet_data = new uint8_t[sizeof(GamePacketOut) + data_size];

    // Build header
    GamePacketOut *packet = new (out_packet_data) GamePacketOut(GamePacketHeader(packet_type, sender_id));

    memcpy(packet->data, packet_data, data_size);

    sockaddr_in to_sockaddr = {};
    to_sockaddr.sin_family = AF_INET;
    to_sockaddr.sin_addr.s_addr = htonl(to.ip);
    to_sockaddr.sin_port = htons(to.port);

    sendto(
        sock,
        (char*)packet,
        (int)(sizeof(GamePacketOut) + data_size),
        0,
        (sockaddr*)&to_sockaddr,
        (int)sizeof(to_sockaddr));

    delete[] out_packet_data;
}

bool recv_game_packet(HbSocket sock, GamePacket *packet, HbSockaddr *from)
{
    sockaddr_in from_sockaddr = {};
    int fromlen = (int)sizeof(from_sockaddr);

    int n = recvfrom(
        sock,
        (char*)packet,
        sizeof(*packet),
        0,
        (sockaddr*)&from_sockaddr,
        &fromlen);

    from->ip = ntohl(from_sockaddr.sin_addr.s_addr);
    from->port = ntohs(from_sockaddr.sin_port);

    return n != -1;     // returns false if none available (or error)
}
