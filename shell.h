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


#define TRUE 1
#define FALSE 0


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


#endif //SHELL_H
