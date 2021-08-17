cmake_minimum_required(VERSION 3.20)
project(test2 C)

set(CMAKE_C_STANDARD 99)

add_executable(test2 main.c application.c application.h semistack.h semistack.c list.c list.h)