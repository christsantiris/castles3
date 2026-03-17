cmake_minimum_required(VERSION 3.16)
project(Castles2)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(castles2 src/main.cpp src/ui.cpp src/map.cpp src/game.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR}) # add data folder to build directory

target_link_libraries(castles2 SDL2::SDL2 SDL2_ttf::SDL2_ttf nlohmann_json::nlohmann_json)