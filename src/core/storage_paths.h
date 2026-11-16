#pragma once
#include <filesystem>
#include <string>

namespace Storage {
    // Mutable saves and scores. The bundled map stays in data/.
#ifdef __EMSCRIPTEN__
    inline constexpr const char* DIRECTORY = "/castles3-saves";
#else
    inline constexpr const char* DIRECTORY = "saves";
#endif

    inline std::string path(const std::string& filename) {
        std::filesystem::create_directories(DIRECTORY);
        return std::string(DIRECTORY) + "/" + filename;
    }
}
