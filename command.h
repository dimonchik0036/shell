/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef COMMAND_H
#define COMMAND_H


#include "shell.h"


#define MAX_ARGS 256
#define MAX_COMMANDS 32
#define MAX_COMMAND_LINE 1024

#define EMPTY 0
#define IN_PIPE 1
#define OUT_PIPE 2
#define BACKGROUND 4
#define IN_FILE 8
#define OUT_FILE 16


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


Command *command_copy(Command const *command);

void command_free(Command *command);

char *command_get_string(Command *command);


#endif //COMMAND_H
