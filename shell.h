#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define MAX_ARGS 256
#define MAX_COMMANDS 32
#define MAX_COMMAND_LINE 1024

#define EMPTY 0
#define IN_PIPE 1
#define OUT_PIPE 2
#define BACKGROUND 4
#define IN_FILE 8
#define OUT_FILE 16

#define EXIT 1
#define CONTINUE 0
#define CRASH (-1)
#define STOP 2

#define BAD_RESULT (-1)
#define BAD_SYNTAX (-1)

#define BAD_PID (-1)
#define DESCENDANT_PID 0

#define TRUE 1
#define FALSE 0

#define PROMPT_LINE "(*_*)$>"


struct Command_St {
    char *arguments[MAX_ARGS];
    char flag;
    char *infile;
    char *outfile;
    char appfile;
};

typedef struct Command_St Command;

struct CommandLine_St {
    Command commands[MAX_COMMANDS];
    pid_t pids[MAX_COMMANDS];
    int start_pipeline_index;
    int current_pipeline_index;
    int prev_out_pipe;
    int pipe_des[2];
};

typedef struct CommandLine_St CommandLine;


ssize_t prompt_line(char *line, size_t line_size);

int parse_input_line(char *line, CommandLine *command_line);

int execute_command_line(CommandLine *command_line, int number_of_commands);


#endif //SHELL_H
