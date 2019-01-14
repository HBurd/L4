#include "hb/net.h"
#include "hb/packets.h"
#include "hb/ship.h"

#ifdef __unix__
#include "net_unix.cpp"
#endif

#ifdef _WIN32
#include "net_windows.cpp"
#endif

// (currently everything is platform specific)
