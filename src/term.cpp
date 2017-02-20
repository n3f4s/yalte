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

// UNIX includes
#include <cstdlib>
#include <pty.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "echo_disable.hpp"
#include "fd.hpp"
#include "guard.hpp"
#include "pty.hpp"
#include "linux.hpp"

// TODO : mettre les externes qui vont bien
// TODO : refactoring
// TODO : send to git
// TODO : handle key
// FIXME : do not work well with htop
// FIXME : handle arrow key

void my_handler(int)
{
    //std::cout << "Exiting\n";
    //exit(1);
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

int main()
{
    // Disable echo on STDIN; data sent to
    // the shell will be read back together
    // with shell output. Also, data is read
    // immediately, without waiting for a
    // delimiter.

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    termios oldt;
    termios newt;
    make_guard(
        [&oldt, &newt]() {
            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_oflag &= ~(ECHO | ECHONL | ICANON);
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &newt);
        },
        [&oldt]() {
            oldt.c_lflag |= ECHO | ECHONL | ICANON;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldt);
        });

    auto fd = pty::fork_term();

    if (fd) {
        bool run = true;

        char buf_stdin, buf_master;
        auto const& master = fd->as_int();

        timeval tv{ 1, 100000 };
        // FIXME : forward signal to shell
        // FIXME : find a way to avoid taking 100% of a CPU
        auto loop_time = 0.1ms;
        while (run) {
            auto begin_ = std::chrono::steady_clock::now();
            stdio_fd_set io_fd;

            FD::set(master, io_fd.read);
            FD::set(STDIN_FILENO, io_fd.read);
            FD::select(master + 1, io_fd.read, io_fd.write, io_fd.except, tv);

            // Talk to the shell
            if (FD::isset(master, io_fd.read)) {
                if (read(master, &buf_master, 1) != -1)
                    write(STDOUT_FILENO, &buf_master, 1);
                else
                    run = false;
            }

            // Talk to stdin
            if (FD::isset(STDIN_FILENO, io_fd.read)) {
                if (read(STDIN_FILENO, &buf_stdin, 1) == 0) {
                    buf_stdin = EOF;
                    write(master, &buf_stdin, 1);
                    return 0;
                }
                switch (buf_stdin) {
                    case char(linux::special_key::up):
                    case char(linux::special_key::down):
                    case char(linux::special_key::left):
                    case char(linux::special_key::right):
                    case char(linux::special_key::escape):
                    default:
                        write(master, &buf_stdin, 1);
                }
            }
            auto end_ = std::chrono::steady_clock::now();
            auto elapsed = end_ - begin_;
            if (elapsed < loop_time)
                std::this_thread::sleep_for(loop_time - elapsed); // FIXME : not the best solution
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
