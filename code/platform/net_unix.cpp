#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include <cassert>

#include "hb/net.h"
#include "hb/packets.h"

HbSocket create_game_socket()
{
    // Create the socket
    HbSocket sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);
    
    // Set it to nonblocking
    int nonblocking = 1;
    assert(ioctl(sock, FIONBIO, &nonblocking) >= 0);

    return sock;
}

void bind_game_socket(HbSocket sock, uint16_t port)
{
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    assert(bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) >= 0);
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
        packet,
        sizeof(GamePacketOut) + data_size,
        0,
        (sockaddr*)&to_sockaddr,
        sizeof(to_sockaddr));

    delete[] out_packet_data;
}

bool recv_game_packet(HbSocket sock, GamePacket *packet, HbSockaddr *from)
{
    sockaddr_in from_sockaddr = {};
    size_t fromlen = sizeof(from_sockaddr);

    int n = recvfrom(
        sock,
        packet,
        sizeof(*packet),
        0,
        (sockaddr*)&from_sockaddr,
        (socklen_t*)&fromlen);

    from->ip = ntohl(from_sockaddr.sin_addr.s_addr);
    from->port = ntohs(from_sockaddr.sin_port);

    return n != -1;     // returns false if none available (or error)
}
