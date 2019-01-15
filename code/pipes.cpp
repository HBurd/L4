#include "hb/pipes.h"
#include <cassert>


#ifdef _WIN32
#include <Windows.h>

HbPipe create_subproc_with_redirected_stdout(char *command_line)
{
    // Security attibutes specify that the created handle should be inherited
    SECURITY_ATTRIBUTES security_attributes = {};
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = nullptr;

    // Create the server side of the pipe (for reading)
    HANDLE parent_read_pipe = CreateNamedPipe(
        "\\\\.\\pipe\\L4server_pipe",
        PIPE_ACCESS_INBOUND,	// client to server
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        1,
        0,
        256,
        0,
        nullptr);

    // Connect the child side of the pipe (for writing).
    // The child's stdout will be redirected to this handle.
    HANDLE child_write_pipe = CreateFile(
        "\\\\.\\pipe\\L4server_pipe",
        GENERIC_WRITE,
        0,
        &security_attributes,
        OPEN_EXISTING,
        0,
        0);

    // Startup info used by CreateProcess to specify which handles to use
    // for stdin and stdout
    STARTUPINFO startup_info = {};
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.hStdError = child_write_pipe;
    startup_info.hStdOutput = child_write_pipe;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    // Process information returned by CreateProcess
    PROCESS_INFORMATION process_info = {};

    assert(CreateProcess(
        nullptr,
        command_line,
        nullptr,
        nullptr,
        true,
        0,
        nullptr,
        nullptr,
        &startup_info,
        &process_info));

    // Close unneeded handles
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    // Close the handle to the write end of the pipe that the server still owns.
    // Child should retain its copy of the handle.
    CloseHandle(child_write_pipe);

    return parent_read_pipe;
}

bool read_pipe(HbPipe pipe, char *buf, unsigned *len)
{
    DWORD len_read = 0;
    if (ReadFile(pipe, buf, *len, &len_read, 0))
    {
        // something was read
        *len = len_read;
        return true;
    }
    else
    {
        // either nothing was read or there was an error
        DWORD last_error = GetLastError();
        if (last_error == ERROR_NO_DATA)
        {
            *len = 0;
            return true;
        }
        else
        {
            return false;
        }
    }
}
#endif

#ifdef __unix__
#include <unistd.h>
#include <fcntl.h>

HbPipe create_subproc_with_redirected_stdout(const char *path)
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        std::cout << "error creating pipe" << std::endl;
    }
    // create the server process
    if (fork() == 0) // we are child process (server)
    {
        // close the read end of the pipe and remap stdout to write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        char port_arg[6] = {};   // 2^16 has max 5 digits, plus null terminator
        snprintf(port_arg, sizeof(port_arg), "%d", port);
        // replace process with server process
        execl("L4server", "L4server", port_arg, nullptr);
    }
    else // we are parent process
    {
        // close the write end of the pipe and save return
        close(pipefd[1]);
        
        // set the pipe to nonblocking
        fcntl(pipefd[0] F_SETFL, O_NONBLOCK);
        return pipefd[0];
    }
    return; // should never be reached
}

bool read_pipe(HbPipe pipe, char *buf, unsigned *len)
#endif