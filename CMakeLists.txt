cmake_minimum_required(VERSION 3.9)
project(shell C)

set(CMAKE_C_STANDARD 99)

add_executable(shell shell.c shell.h prompt_line.c parse_line.c execute.c)
