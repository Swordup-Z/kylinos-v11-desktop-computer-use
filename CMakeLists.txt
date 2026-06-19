cmake_minimum_required(VERSION 3.16)

project(kylinos-v11-desktop-computer-use VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED COMPONENTS Xtst)

add_executable(kylinos-v11-desktop-computer-use
    src/main.cpp
)

target_link_libraries(kylinos-v11-desktop-computer-use PRIVATE
    X11::X11
    X11::Xtst
)
