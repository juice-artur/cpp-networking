cmake_minimum_required(VERSION 3.14)
project(UDPChat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory(Client)
add_subdirectory(Server)
