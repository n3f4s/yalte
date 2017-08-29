#pragma once

#include <string>
#include <experimental/optional>
#include <functional>
#include <cstring>

extern "C" {
#include <unistd.h>
#include <termios.h>
#include <signal.h>
}

namespace linux {
    enum class special_key : char {
        up     = 72,
        down   = 80,
        left   = 75,
        right  = 77,
        escape = 27,
    }; // Not really usefull

    enum class Error : unsigned {
        EPERM   = 1,
        ENOENT  = 2,
        ESRCH   = 3,
        EINTR   = 4,
        EIO     = 5,
        ENXIO   = 6,
        E2BIG   = 7,
        ENOEXEC = 8,
        EBADF   = 9,
        ECHILD  = 10,
        EAGAIN  = 11,
        ENOMEM  = 12,
        EACCES  = 13,
        EFAULT  = 14,
        ENOTBLK = 15,
        EBUSY   = 16,
        EEXIST  = 17,
        EXDEV   = 18,
        ENODEV  = 19,
        ENOTDIR = 20,
        EISDIR  = 21,
        EINVAL  = 22,
        ENFILE  = 23,
        EMFILE  = 24,
        ENOTTY  = 25,
        ETXTBSY = 26,
        EFBIG   = 27,
        ENOSPC  = 28,
        ESPIPE  = 29,
        EROFS   = 30,
        EMLINK  = 31,
        EPIPE   = 32,
        EDOM    = 33,
        ERANGE  = 34,
        OK      = 35
    };

    Error errno_to_enum() {
      static Error errors[] =
        { Error::EPERM   , Error::ENOENT , Error::ESRCH  , Error::EINTR , Error::EIO,
          Error::ENXIO   , Error::E2BIG  , Error::ENOEXEC, Error::EBADF , Error::ECHILD,
          Error::EAGAIN  , Error::ENOMEM , Error::EACCES , Error::EFAULT, Error::ENOTBLK,
          Error::EBUSY   , Error::EEXIST , Error::EXDEV  , Error::ENODEV, Error::ENOTDIR,
          Error::EISDIR  , Error::EINVAL , Error::ENFILE , Error::EMFILE, Error::ENOTTY,
          Error::ETXTBSY , Error::EFBIG  , Error::ENOSPC , Error::ESPIPE, Error::EROFS,
          Error::EMLINK  , Error::EPIPE  , Error::EDOM   , Error::ERANGE, Error::OK };
        return errors[errno];
    }

    Error chdir(const std::string& path) {
        if(chdir(path.c_str()) == -1) {
            return errno_to_enum();
        }
        return Error::OK;
    }
    using std::experimental::optional;
    optional<std::string> getenv(const std::string& env) {
        auto res = ::getenv(env.c_str());
        if(res) return optional<std::string>(std::string(res, std::strlen(res)));
        return optional<std::string>{};
    }
    template<typename... T>
    int execlp(const std::string& cmd, const std::string& path, const T&... args) {
        return ::execlp(cmd.c_str(), path.c_str(), args..., nullptr);
    }

    ::termios get_termios(int fd) {
        ::termios t;
        ::tcgetattr(fd, &t);
        return t;
    }

    // TODO : replace C termios by this termios and test
    // FIXME : non copiable but movable (not in assignement) (maybe copiable)
    struct termios {
        private:
            int fd;
            ::termios old;
        public:
        ::termios term;

        tcflag_t& c_iflag;      /* input modes */
        tcflag_t& c_oflag;      /* output modes */
        tcflag_t& c_cflag;      /* control modes */
        tcflag_t& c_lflag;      /* local modes */
        cc_t     (&c_cc)[NCCS];   /* special characters */


        termios(int fd_):
            fd{fd_},
            old{get_termios(fd)},
            term{old},
            c_iflag{old.c_iflag},
            c_oflag{term.c_oflag},
            c_cflag{term.c_cflag},
            c_lflag{term.c_lflag},
            c_cc{term.c_cc}
        {
            tcgetattr(fd, &term);
            tcgetattr(fd, &old); // Copy should not cause issue but to be sure
        }

        ~termios() {
            tcsetattr(fd, TCSAFLUSH, &old);
        }

    };

    void tcsetattr(int fd, int option, termios& t) {
        ::tcsetattr(fd, option, &(t.term));
    }

}
