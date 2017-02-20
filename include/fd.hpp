#pragma once

#include <experimental/optional>

extern "C" {
#include <sys/select.h>
}

using std::experimental::optional;
namespace FD {
class FileDescriptor {
public:
    FileDescriptor(int fd, bool closeable = true) noexcept;

    FileDescriptor(FileDescriptor const&) = delete;
    FileDescriptor(FileDescriptor&& other) noexcept;

    FileDescriptor& operator=(FileDescriptor const&) = delete;
    FileDescriptor& operator=(FileDescriptor&& other) noexcept;

    void premature_close() noexcept;

    int& as_int() noexcept;

    ~FileDescriptor() noexcept;

private:
    int file_desc;
    bool is_closeable;
};
class Set {
public:
    Set()
    {
        FD_ZERO(&set);
    }

private:
    friend void set(int fd, Set& set);
    // TODO : mettre timeval optional (overload ...)
    friend void select(int fd, optional<Set>& read, optional<Set>& write, optional<Set>& except, timeval& tv);
    friend void select(int fd, Set& read, Set& write, Set& except, timeval& tv);
    friend bool isset(int fd, Set& set);
    fd_set set;
};
void set(int fd, Set& set);
void select(int fd, Set& read, Set& write, Set& except, timeval& tv);
void select(int fd, optional<Set>& read, optional<Set>& write, optional<Set>& except, timeval& tv);
bool isset(int fd, Set& set);
}
