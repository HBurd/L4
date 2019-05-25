#include "hb/net.h"
#include "hb/packets.h"

bool HbSockaddr::operator==(const HbSockaddr &rhs) const
{
    return port == rhs.port && ip == rhs.ip;
}

#ifdef __unix__
#include "platform/net_unix.cpp"
#endif

#ifdef _WIN32
#include "platform/net_windows.cpp"
#endif
