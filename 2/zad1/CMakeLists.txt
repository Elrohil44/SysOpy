cmake_minimum_required(VERSION 3.6)
project(libsysBench)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wpedantic")

set(SOURCE_FILES main.c bench.h bench.c)
add_executable(main ${SOURCE_FILES})

enable_testing()
add_test(test ./test)
