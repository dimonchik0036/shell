/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "command.h"

static char *string_copy(char const *str);


Command *command_copy_for_job(Command const *command) {
    Command *new_command = malloc(sizeof(Command));
    check_memory(new_command);

    new_command->flag = command->flag;
    new_command->appfile = command->appfile;
    new_command->outfile = string_copy(command->outfile);
    new_command->infile = string_copy(command->infile);
    new_command->arguments[0] = string_copy(command->arguments[0]);

    char *arguments = NULL;
    size_t arguments_len = 0;

    int index;
    for (index = 1; command->arguments[index]; ++index) {
        char *current_argument = command->arguments[index];
        size_t current_len = strlen(current_argument);

        arguments = realloc(arguments, (arguments_len + current_len + 1) * sizeof(char));
        check_memory(arguments);

        arguments[arguments_len] = ' ';
        if (!arguments_len) {
            memcpy(arguments, current_argument, current_len * sizeof(char));
        } else {
            memcpy(arguments + arguments_len + 1,
                   current_argument,
                   current_len * sizeof(char));
        }

        arguments_len += current_len;
        arguments[arguments_len] = '\0';
    }

    new_command->arguments[1] = arguments;
    new_command->arguments[2] = NULL;

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

char *command_get_name(Command *command) {
    return command->arguments[0] ? command->arguments[0] : "";
}

char *command_get_args(Command *command) {
    return command->arguments[1] ? command->arguments[1] : "";
}

static char *string_copy(char const *str) {
    if (!str) {
        return NULL;
    }

    return strdup(str);
}
