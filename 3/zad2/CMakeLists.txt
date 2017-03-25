cmake_minimum_required(VERSION 3.6)
project (BATCH_INTERPRETER)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(shell shell.c)
add_library(bench SHARED bench.c)
add_executable(expensive expensive.c)

target_link_libraries(shell bench)
