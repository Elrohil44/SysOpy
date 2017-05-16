cmake_minimum_required(VERSION 3.6)
project (RDWR)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(main main.c)

target_link_libraries(main -lrt -lpthread)
