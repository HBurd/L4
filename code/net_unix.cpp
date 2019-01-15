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
    assert(ioctl(sock, FIONBIO, &nonblocking) >= 0);

    return sock;
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

    sockaddr_in to_sockaddr = {};
    to_sockaddr.sin_family = AF_INET;
    to_sockaddr.sin_addr.s_addr = htonl(to.ip);
    to_sockaddr.sin_port = htons(to.port);

    sendto(
        sock,
        &packet,
        sizeof(header) + data_size,
        0,
        (sockaddr*)&to_sockaddr,
        sizeof(to_sockaddr));
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
