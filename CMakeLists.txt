cmake_minimum_required(VERSION 3.16)
project(Castles3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

if(EMSCRIPTEN)
    # Use portable headers and Emscripten's SDL ports, never host libraries.
    include(FetchContent)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(nlohmann_json REQUIRED)
    find_package(Catch2 3 REQUIRED)
endif()

# Core static library (pure C++, zero SDL)
file(GLOB_RECURSE CORE_SOURCES src/core/*.cpp)
add_library(castles3_core STATIC ${CORE_SOURCES})
target_include_directories(castles3_core PUBLIC src/)
target_link_libraries(castles3_core nlohmann_json::nlohmann_json)

# Game executable
file(GLOB RENDERER_SOURCES src/renderer/*.cpp)
file(GLOB INPUT_SOURCES src/input/*.cpp)
add_executable(castle3
    src/main.cpp
    ${RENDERER_SOURCES}
    ${INPUT_SOURCES}
)
target_link_libraries(castle3 PRIVATE castles3_core)

if(EMSCRIPTEN)
    # JSON parsing and save loading rely on C++ exceptions. Asyncify retains
    # main's C++ stack while the browser's animation clock advances each frame.
    target_compile_options(castles3_core PRIVATE -fexceptions)
    target_compile_options(castle3 PRIVATE -fexceptions -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2)
    target_link_options(castle3 PRIVATE
        -fexceptions
        -sDISABLE_EXCEPTION_CATCHING=0
        -sUSE_SDL=2
        -sUSE_SDL_TTF=2
        -sUSE_SDL_MIXER=2
        "-sSDL2_MIXER_FORMATS=['ogg']"
        -sASYNCIFY=1
        -sASYNCIFY_STACK_SIZE=131072
        -sSTACK_SIZE=8388608
        -sALLOW_MEMORY_GROWTH=1
        -sENVIRONMENT=web
        "-sEXPORTED_RUNTIME_METHODS=['callMain','FS','IDBFS','addRunDependency','removeRunDependency','UTF8ToString','stringToNewUTF8']"
        -lidbfs.js
        "SHELL:--preload-file \"${CMAKE_SOURCE_DIR}/data/map.json@/data/map.json\""
        "SHELL:--preload-file \"${CMAKE_SOURCE_DIR}/fonts@/fonts\""
        "SHELL:--preload-file \"${CMAKE_SOURCE_DIR}/music@/music\""
        --shell-file "${CMAKE_SOURCE_DIR}/package/browser/shell.html"
    )
    file(GLOB_RECURSE BROWSER_ASSETS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/fonts/*" "${CMAKE_SOURCE_DIR}/music/*")
    set_target_properties(castle3 PROPERTIES
        OUTPUT_NAME index
        SUFFIX .html
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/browser"
        LINK_DEPENDS "${CMAKE_SOURCE_DIR}/package/browser/shell.html;${CMAKE_SOURCE_DIR}/data/map.json;${BROWSER_ASSETS}"
    )
    file(WRITE "${CMAKE_SOURCE_DIR}/dist/browser/.nojekyll" "")
else()
    target_link_libraries(castle3 PRIVATE SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer)
endif()

# Native assets and tests are separate from the browser bundle.
if(NOT EMSCRIPTEN)
    if(EXISTS ${CMAKE_SOURCE_DIR}/data)
        file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
    endif()
    if(EXISTS ${CMAKE_SOURCE_DIR}/fonts)
        file(COPY ${CMAKE_SOURCE_DIR}/fonts DESTINATION ${CMAKE_BINARY_DIR})
    endif()
    if(EXISTS ${CMAKE_SOURCE_DIR}/music)
        file(COPY ${CMAKE_SOURCE_DIR}/music DESTINATION ${CMAKE_BINARY_DIR})
    endif()

    # Test executable
    file(GLOB TEST_SOURCES tests/*.cpp)
    add_executable(castles3_tests ${TEST_SOURCES})
    target_include_directories(castles3_tests PRIVATE src/)
    target_link_libraries(castles3_tests
        castles3_core
        Catch2::Catch2WithMain
    )

    enable_testing()
    add_test(NAME castles3_tests COMMAND castles3_tests)
endif()
