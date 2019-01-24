cmake_minimum_required(VERSION 3.12)
project(chat_room C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c server.h wrapper.c wrapper.h)
add_executable(client client.c wrapper.c wrapper.h client.h)
find_package(Threads REQUIRED)
target_link_libraries(server Threads::Threads)
target_link_libraries(client Threads::Threads)
