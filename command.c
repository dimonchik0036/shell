/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "command.h"

static char *string_copy(char const *str);


Command *command_copy(Command *command) {
    Command *new_command = malloc(sizeof(Command));
    check_memory(new_command);

    new_command->flag = command->flag;
    new_command->appfile = command->appfile;
    new_command->outfile = string_copy(command->outfile);
    new_command->infile = string_copy(command->infile);

    int index;
    for (index = 0; command->arguments[index]; ++index) {
        new_command->arguments[index] = string_copy(command->arguments[index]);
    }

    new_command->arguments[index] = NULL;

    return new_command;
}

void command_free(Command *command) {
    if (!command) {
        return;
    }

    free(command->infile);
    free(command->outfile);

    int index;
    for (index = 0; command->arguments[index]; ++index) {
        free(command->arguments[index]);
    }

    free(command);
}

static char *string_copy(char const *str) {
    if (!str) {
        return NULL;
    }

    return strdup(str);
}
