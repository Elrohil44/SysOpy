cmake_minimum_required(VERSION 3.6)
project (SOCKETS)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server -lrt -lpthread)
target_link_libraries(client -lrt -lpthread)
