// C++ includes
#include <chrono>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <fstream> // FIXME for logging
#include <cctype>

// UNIX includes
#include <cstdlib>
#include <pty.h>
#include <signal.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "echo_disable.hpp"
#include "fd.hpp"
#include "guard.hpp"
#include "pty.hpp"
#include "linux.hpp"
#include "debug.hpp"

#ifndef ASCII_DEBUG
#define ASCII_DEBUG(val)
#endif /* ifndef ASCII_DEBUG(val) */

// TODO : mettre les externes qui vont bien
// TODO : refactoring
// TODO : send to git
// TODO : handle key
// FIXME : do not work well with htop
// FIXME : handle arrow key
// FIXME : forward signal to shell
// FIXME : find a way to avoid taking 100% of a CPU (resolv with waiting with select ?)
// FIXME : better debug logging

void my_handler(int)
{
    // std::cout << "Exiting\n";
    exit(1);
}

// TODO : généraliser en split
std::string get_shell_name(std::string const& path)
{
    std::string tmp;
    tmp.reserve(path.size());
    for (auto const& c : path) {
        if (c == '/') {
            tmp.clear();
        } else {
            tmp.push_back(c);
        }
    }
    return tmp;
}

using namespace std::literals;

struct stdio_fd_set {
    FD::Set read;
    FD::Set write;
    FD::Set except;
};

// TODO : clean code

int main()
{
    struct sigaction sigIntHandler; // FIXME maybe C++ify sigaction

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // termios oldt; // Make a good C++ interface
    // termios newt;
    linux::termios term{STDIN_FILENO};
    term.c_oflag &= ~(ECHO | ECHONL | ICANON);
    linux::tcsetattr(STDIN_FILENO, TCSAFLUSH, term);

    auto fd = pty::fork_term();

    std::ofstream log_in{"./tmp-in.log"};
    std::ofstream log_out{"./tmp-out.log"};
    if (fd) {
        bool run = true;

        unsigned char buf_stdin, buf_master;
        auto const& master = fd->first.as_int();

        while (run) {
            stdio_fd_set io_fd;

            FD::set(master, io_fd.read);
            FD::set(STDIN_FILENO, io_fd.read);
            FD::select(master + 1, io_fd.read, io_fd.write, io_fd.except);

            // Talk to the shell
            if (FD::isset(master, io_fd.read)) {
                if (read(master, &buf_master, 1) != -1) {
                    write(STDOUT_FILENO, &buf_master, 1);
                    log_out << ASCII_DEBUG(buf_master) << std::flush;
                } else {
                    run = false;
                }
            }

            // Talk to stdin
            if (FD::isset(STDIN_FILENO, io_fd.read)) {
                if (read(STDIN_FILENO, &buf_stdin, 1) == 0) {
                    buf_stdin = EOF;
                    write(master, &buf_stdin, 1);
                    return 0;
                }
                if(buf_stdin == 'Q') {
                    //buf_stdin = 24;
                    //buf_stdin = '\0';
                    kill(fd->second, SIGINT);
                }
                else {
                    write(master, &buf_stdin, 1);
                }
                log_in << ASCII_DEBUG(buf_stdin);
            }
        }
    } else {
        const std::string shell_path = "/bin/bash"; //getenv("SHELL");
        const std::string shell_name = get_shell_name(shell_path);
        if (chdir(getenv("HOME")) == -1)
            return 254; // TODO : notify master -> send EOF ???
        if (execlp(shell_name.c_str(), shell_path.c_str(), NULL) == -1) {
            return 253;
        }
    }

    return 0;
}
