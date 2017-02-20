
#include "pty.hpp"

#include <stdexcept>

#include <cstdlib>
#include <termios.h>
#include <unistd.h>

namespace pty {
std::string _concat_string(char* r, char* l)
{
    // Ok, maybe it's not very efficient (for now ^^)
    return std::string{ r } + std::string{ l };
}
std::string _concat_string(const char* r, char* l)
{
    // Ok, maybe it's not very efficient (for now ^^)
    return std::string{ r } + std::string{ l };
}
void openpty(int& master, int& slave)
{
    if (::openpty(&master, &slave, nullptr, nullptr, nullptr) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, char* name)
{
    if (::openpty(&master, &slave, name, nullptr, nullptr) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, termios& termp)
{
    if (::openpty(&master, &slave, nullptr, &termp, nullptr) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, winsize& winp)
{
    if (::openpty(&master, &slave, nullptr, nullptr, &winp) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, char* name, termios& termp)
{
    if (::openpty(&master, &slave, name, &termp, nullptr) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, char* name, termios& termp, winsize& winp)
{
    if (::openpty(&master, &slave, name, &termp, &winp) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, char* name, winsize& winp)
{
    if (::openpty(&master, &slave, name, nullptr, &winp) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
void openpty(int& master, int& slave, termios& termp, winsize& winp)
{
    if (::openpty(&master, &slave, nullptr, &termp, &winp) != 0)
        throw std::runtime_error(_concat_string("opentty failed", strerror(errno)));
}
optional<FD::FileDescriptor> fork_term()
{
    FD::FileDescriptor master{ 0 };
    FD::FileDescriptor slave{ 0 };
    pty::openpty(master.as_int(), slave.as_int());
    pid_t pid = 0;
    switch (pid = fork()) {
    case -1:
        throw std::runtime_error("Fork failed");
        break;
    case 0:
        // TODO : use login_tty instead ?
        setsid();
        dup2(slave.as_int(), STDIN_FILENO);
        dup2(slave.as_int(), STDOUT_FILENO);
        dup2(slave.as_int(), STDERR_FILENO);
        return optional<FD::FileDescriptor>{};
    default:
        // TODO : handle sigchld signal
        return optional<FD::FileDescriptor>(std::move(master));
    }
    return optional<FD::FileDescriptor>{};
}
}
