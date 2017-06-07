cmake_minimum_required(VERSION 3.6)
project (SOCKETS)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(load load.c)
add_executable(example example.c)

add_library(trace SHARED tracer.c tracee.c)

target_link_libraries(load trace)
target_link_libraries(example -lrt -lpthread)
