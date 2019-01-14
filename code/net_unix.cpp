#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "hb/net.h"

HbSocket create_game_socket()
{
    // Create the socket
    HbSocket sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);
    
    // Set it to nonblocking
    int nonblocking = 1;
    ioctl(sock, FIONBIO, nonblocking);

    return sock;
}

HbSockaddr create_sockaddr(uint32_t ip, uint16_t port)
{
    HbSockaddr result;
    result.sin_family = AF_INET;
    result.sin_addr.s_addr = htonl(ip);
    result.sin_port = htons(port);

    return result;
}

void send_game_packet(
    HbSocket sock,
    HbSockaddr to,
    ClientId sender_id,
    GamePacketType packet_type,
    void *packet_data,
    size_t data_size)
{
    // Build header
    GamePacketHeader header(packet_type, sender_id);
    GamePacketOut packet(header, packet_data, data_size);
    sendto(
        sock,
        &packet,
        sizeof(header) + data_size,
        0,
        (sockaddr*)&to,
        sizeof(to));
}

bool recv_game_packet(HbSocket sock, GamePacket *packet, HbSockaddr *from)
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
