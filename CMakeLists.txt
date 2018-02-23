cmake_minimum_required(VERSION 3.9)
project(shell C)

set(CMAKE_C_STANDARD 99)

add_executable(shell
               shell.c
               shell.h
               prompt_line.c
               prompt_line.h
               parse_line.c
               parse_line.h
               execute.c
               execute.h
               job_control.c
               job_control.h
               command.c
               command.h
               job.c
               job.h)
