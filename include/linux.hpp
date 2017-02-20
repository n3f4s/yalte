#pragma once

#include <string>
#include <experimental/optional>
#include <cstring>

extern "C" {
#include <unistd.h>
}

namespace linux {
    enum class special_key : char {
        up     = 72,
        down   = 80,
        left   = 75,
        right  = 77,
        escape = 27,
    };

    void chdir(const std::string& path) {
        chdir(path.c_str());
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
}
