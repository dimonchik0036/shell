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
    int prev_out_pipe;
    int pipe_des[2];
    size_t current_index_of_command;
    size_t last_command_in_pipeline;
};

typedef struct CommandLine_St CommandLine;


Command *command_copy_for_job(const Command *command);

void command_free(Command *command);

char *command_get_args(const Command *command);

char *command_get_name(const Command *command);

Command *command_concat(const Command *commands, size_t count);


#endif //COMMAND_H
