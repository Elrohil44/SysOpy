project (CONTACTBOOK)

set(CMAKE_C_FLAGS "--std=gnu11 ${CMAKE_C_FLAGS}")
add_library (shared SHARED lib/contactBook.c)
add_library (static STATIC lib/contactBook.c)
add_library (dynamic MODULE lib/contactBook.c)

add_executable(main_static main.c)
add_executable(main_dynamic maindynamic.c)
add_executable(main_shared main.c)

target_link_libraries(main_static static)
target_link_libraries(main_shared shared)
target_link_libraries(main_dynamic ${CMAKE_DL_LIBS})