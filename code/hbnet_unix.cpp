#include <sys/socket.h>
#include <netinet/in.h>

#include "hbnet.h"

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
