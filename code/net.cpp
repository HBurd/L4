#include "hb/net.h"
#include "hb/packets.h"
#include "hb/ship.h"

bool HbSockaddr::operator==(const HbSockaddr &rhs) const
{
    return port == rhs.port && ip == rhs.ip;
}

#ifdef __unix__
#include "net_unix.cpp"
#endif

#ifdef _WIN32
#include "net_windows.cpp"
#endif
