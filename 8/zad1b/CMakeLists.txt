cmake_minimum_required(VERSION 3.6)
project (MULTITHREADING)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(main main.c)
add_executable(gen gen.c)

target_link_libraries(main -lrt -lpthread)
