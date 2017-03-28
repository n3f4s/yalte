// C++ includes
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <fstream> // FIXME for logging
#include <experimental/optional>

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
#ifndef NDEBUG
#include "debug.hpp"
#endif

#ifndef ASCII_DEBUG
#define ASCII_DEBUG(val)
#endif /* ifndef ASCII_DEBUG(val) */

// ctrl+c => SIGINT
// ctrl+z => SIGTSTP
// ctrl+\ => SIGQUIT
// ctrl+t => SIGINFO

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

// TODO : clean code

std::ofstream log_in{"./tmp-in.log"};
std::ofstream log_out{"./tmp-out.log"};

pid_t shell_pid;

bool talk_to_shell(int master, stdio_fd_set const& fds) {
    if(!FD::isset(master, fds.read)) return true;
    unsigned char buf;
    if(read(master, &buf, 1) == -1) return false;
    write(STDOUT_FILENO, &buf, 1);
    return true;
}

std::experimental::optional<unsigned char> parse_stdin(unsigned char buf) {
    switch (buf) {
        case 'Q':
            kill(shell_pid, SIGINT);
            return {};
        default:
            return buf;
    }
    return {};
}

bool talk_to_stdin(int master, stdio_fd_set const& fds) {
    if(!FD::isset(STDIN_FILENO, fds.read)) return true;
    unsigned char buf;
    switch (read(STDIN_FILENO, &buf, 1)) {
        case -1:
            return false;
            break;
        case 0:
            buf = EOF;
            write(master, &buf, 1);
            return false;
            break;
        default:
            log_in << ASCII_DEBUG(buf);
            if(auto buf_ = parse_stdin(buf)) { // FIXME can parse_stdin makes the terminal exit ?
                write(master, &(*buf_), 1);
            }
            return true;
    }
}

int main()
{
    struct sigaction sigIntHandler; // FIXME maybe C++ify sigaction

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    linux::termios term{STDIN_FILENO};
    term.c_lflag &= ~(ECHO | ECHONL | ICANON);
    linux::tcsetattr(STDIN_FILENO, TCSAFLUSH, term);

    auto fd = pty::fork_term();

    if (fd) {
        bool run = true;

        auto const& master = fd->first.as_int();
        shell_pid = fd->second;

        while (run) {
            stdio_fd_set io_fd;

            FD::set(master, io_fd.read);
            FD::set(STDIN_FILENO, io_fd.read);
            FD::select(master + 1, io_fd.read, io_fd.write, io_fd.except);

            // Talk to the shell
            run = talk_to_shell(master, io_fd);

            // Talk to stdin
            run &= talk_to_stdin(master, io_fd);
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
