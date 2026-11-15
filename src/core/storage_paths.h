#pragma once
#include <string>

namespace Storage {
    // Only mutable data lives in IDBFS. The bundled map stays in /data.
#ifdef __EMSCRIPTEN__
    inline constexpr const char* DIRECTORY = "/castles3-saves";
#else
    inline constexpr const char* DIRECTORY = "data";
#endif

    inline std::string path(const std::string& filename) {
        return std::string(DIRECTORY) + "/" + filename;
    }
}
