cmake_minimum_required(VERSION 3.6)
project (BARBER)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

add_executable(barber barber.c)
add_executable(clients clients.c)
