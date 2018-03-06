/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "command.h"

static char *string_copy(char const *str);

static char *string_create(size_t len, size_t capacity);

static char *string_concat(char *dst,
                           char const *src,
                           size_t *dst_capacity,
                           size_t *dst_len);

static char *command_copy_arguments(Command const *command,
                                    size_t offset,
                                    char *dst,
                                    size_t *dst_capacity,
                                    size_t *dst_len);

static Command *command_base_copy(Command const *command);

Command *command_copy_for_job(Command const *command) {
    Command *new_command = command_base_copy(command);
    new_command->arguments[0] = string_copy(command->arguments[0]);

    size_t arguments_len = 0;
    size_t arguments_capacity = 10;
    char *arguments = string_create(arguments_len, arguments_capacity);
    arguments = command_copy_arguments(command, 1, arguments,
                                       &arguments_capacity,
                                       &arguments_len);

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

char *command_get_name(const Command *command) {
    return command->arguments[0] ? command->arguments[0] : "";
}

char *command_get_args(const Command *command) {
    return command->arguments[1] ? command->arguments[1] : "";
}

Command *command_concat(const Command *commands, size_t count) {
    if (count == 0) {
        return NULL;
    }

    size_t last_index = count - 1;
    Command *new_command = command_base_copy(&commands[last_index]);

    size_t command_name_len = 0;
    size_t command_name_capacity = 10;
    char *command_name = string_create(command_name_len, command_name_capacity);

    size_t command_index;
    for (command_index = 0; command_index < count; ++command_index) {
        Command const *current_command = &commands[command_index];
        command_name = command_copy_arguments(current_command, 0, command_name,
                                              &command_name_capacity,
                                              &command_name_len);

        if (command_index != last_index) {
            command_name = string_concat(command_name, " | ",
                                         &command_name_capacity,
                                         &command_name_len);
        }
    }

    new_command->arguments[0] = command_name;
    new_command->arguments[1] = NULL;

    return new_command;
}

static char *string_copy(char const *str) {
    if (!str) {
        return NULL;
    }

    return strdup(str);
}

static char *string_create(size_t len, size_t capacity) {
    char *new_string = malloc(capacity * sizeof(char));
    check_memory(new_string);
    new_string[len] = END;

    return new_string;
}

static char *string_concat(char *dst,
                           char const *src,
                           size_t *dst_capacity,
                           size_t *dst_len) {
    size_t src_len = strlen(src);
    size_t result_len = *dst_len + src_len + 1;
    if (result_len >= *dst_capacity) {
        *dst_capacity = *dst_capacity * 2 > result_len
                        ? *dst_capacity * 2
                        : result_len;
        dst = realloc(dst, *dst_capacity * sizeof(char));
    }

    dst = strcat(dst, src);
    *dst_len += src_len;

    return dst;
}

static char *command_copy_arguments(Command const *command,
                                    size_t offset,
                                    char *dst,
                                    size_t *dst_capacity,
                                    size_t *dst_len) {
    size_t argument_index = offset;
    while (command->arguments[argument_index] != NULL) {
        char *current_argument = command->arguments[argument_index];
        if (argument_index != offset) {
            dst = string_concat(dst, " ", dst_capacity, dst_len);
        }

        dst = string_concat(dst, current_argument, dst_capacity, dst_len);
        ++argument_index;
    }

    return dst;
}

static Command *command_base_copy(Command const *command) {
    Command *new_command = malloc(sizeof(Command));
    check_memory(new_command);

    new_command->flag = command->flag;
    new_command->appfile = command->appfile;
    new_command->outfile = string_copy(command->outfile);
    new_command->infile = string_copy(command->infile);
    return new_command;
}
