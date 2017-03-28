
#include "fd.hpp"

extern "C" {
#include <pty.h>
#include <sys/time.h>
#include <unistd.h>
}

namespace FD {
FileDescriptor::FileDescriptor(int fd, bool closeable) noexcept : file_desc{ fd }, is_closeable{ closeable }
{
}
FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : file_desc{ other.file_desc },
      is_closeable{ other.is_closeable }
{
    other.is_closeable = false;
}
FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) noexcept
{
    file_desc = other.file_desc;
    is_closeable = other.file_desc;
    other.is_closeable = false;
    return *this;
}
void FileDescriptor::premature_close() noexcept
{
    close(file_desc);
    is_closeable = false;
}

int& FileDescriptor::as_int() noexcept
{
    return file_desc;
}

FileDescriptor::~FileDescriptor() noexcept
{
    if (is_closeable)
        close(file_desc);
}
void set(int fd, Set& set)
{
    FD_SET(fd, &(set.set));
}
void select(int fd, Set& read, Set& write, Set& except)
{
    ::select(fd + 1, &read.set, &write.set, &except.set, nullptr);
}
void select(int fd, Set& read, Set& write, Set& except, timeval& tv)
{
    ::select(fd + 1, &read.set, &write.set, &except.set, &tv);
}
void select(int fd, optional<Set>& read, optional<Set>& write, optional<Set>& except, timeval& tv)
{
    ::select(fd + 1, read ? &read->set : nullptr, write ? &write->set : nullptr, except ? &except->set : nullptr, &tv);
}
bool isset(int fd, Set const& set)
{
    return FD_ISSET(fd, &set.set);
}
}
