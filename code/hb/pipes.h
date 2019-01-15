#ifndef HBPIPES_H
#define HBPIPES_H

/* TODO: The implementation of HbPipe's is incomplete!
   I had trouble finding an easy way of specifying command
   line arguments to create_subproc_with_redirected_stdout
   in a way that worked across platforms.
*/

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE HbPipe;
#endif

#ifdef __unix__
typedef int HbPipe;
#endif

HbPipe create_subproc_with_redirected_stdout(const char *path);

// returns false if there was an error
// len is inout
bool read_pipe(HbPipe pipe, char *buf, unsigned *len);

#ifdef FAST_BUILD
#include "pipes.cpp"
#endif

#endif // include guard