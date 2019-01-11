#include <sys/socket.h>
#include <netinet/in.h>

#include "hb/net.h"

void send_game_packet(
    int sock,
    sockaddr_in to,
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

bool recv_game_packet(int sock, GamePacket *packet, sockaddr_in *from)
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
