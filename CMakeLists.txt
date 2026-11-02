cmake_minimum_required(VERSION 4.3)
project(keylogger C)

set(CMAKE_C_STANDARD 99)
add_executable(keylogger keylogger.c
        keylogger.h)
target_link_libraries(keylogger winhttp)