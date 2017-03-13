cmake_minimum_required(VERSION 3.6)
project (CONTACTBOOK)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3 -Wall -Wpedantic")
add_library (shared SHARED lib/contactBook.c)
add_library (static STATIC lib/contactBook.c)
add_library (dynamic MODULE lib/contactBook.c)

add_executable(main_static main.c)
add_executable(main_dynamic maindynamic.c)
add_executable(main_shared main.c)
add_executable(coredump maincoredump.c)

target_link_libraries(main_static static)
target_link_libraries(main_shared shared)
target_link_libraries(main_dynamic ${CMAKE_DL_LIBS})
target_link_libraries(coredump static)

enable_testing()
add_test(dynamic ./main_dynamic)
add_test(shared ./main_shared)
add_test(static ./main_static)