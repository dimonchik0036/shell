/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "parse_line.h"


#define PRINT_SYNTAX_ERROR(token) fprintf(stderr, "shell: syntax error near unexpected token '%s'\n", token)

static char *blank_skip(char *data);

static int parse_redirect_output(char **data, Command *command);

static int parse_redirect_input(char **data, Command *command);

static int parse_background(char **data,
                            CommandLine *command_line,
                            size_t *current_index_of_command,
                            size_t *current_index_of_arguments);

static int parse_concat(char **data,
                        CommandLine *command_line,
                        size_t *current_index_of_command,
                        size_t *current_index_of_arguments);

static int parse_separator(char **data,
                           size_t *current_index_of_command,
                           size_t *current_index_of_arguments);

static int parse_add_command(char **data,
                             Command *current_command,
                             const size_t *current_index_of_command,
                             size_t *current_index_of_arguments,
                             size_t *number_of_command);

static int parse_tokens(char **data,
                        CommandLine *command_line,
                        size_t *current_index_of_command,
                        size_t *current_index_of_arguments,
                        size_t *number_of_command);

static void go_to_next_delimiter(char **data);

static void set_end(char **data);

static void reset_command_line(CommandLine *command_line);

inline int is_end(char const *data) {
    return *data == END;
}


static char delimiters[] = " \t&<>;\n";


ssize_t parse_input_line(char *input_data, CommandLine *command_line) {
    reset_command_line(command_line);
    size_t index_of_arguments = 0;
    size_t index_of_command = 0;

    size_t result_number_of_command = 0;
    while (!is_end(input_data)) {
        if (index_of_command >= MAX_COMMANDS) {
            fprintf(stderr, "shell: number of commands (%d) exceeded\n",
                    MAX_COMMANDS);
            return BAD_SYNTAX;
        }

        input_data = blank_skip(input_data);
        if (is_end(input_data)) {
            break;
        }

        int exit_code = parse_tokens(&input_data, command_line,
                                     &index_of_command,
                                     &index_of_arguments,
                                     &result_number_of_command);
        if (exit_code == BAD_SYNTAX) {
            return exit_code;
        }
    }

    if (result_number_of_command > 0) {
        if (command_line->commands[result_number_of_command - 1].flag
            & OUT_PIPE) {
            PRINT_SYNTAX_ERROR(TOKEN_PIPELINE_STR);
            return BAD_SYNTAX;
        }
    }

    return result_number_of_command;
}

static int parse_tokens(char **data,
                        CommandLine *command_line,
                        size_t *current_index_of_command,
                        size_t *current_index_of_arguments,
                        size_t *number_of_command) {
    Command *current_command = &command_line->commands[*current_index_of_command];

    char token = **data;
    switch (token) {
        case TOKEN_BACKGROUND:
            return parse_background(data, command_line,
                                    current_index_of_command,
                                    current_index_of_arguments);
        case TOKEN_OUTFILE:
            return parse_redirect_output(data, current_command);
        case TOKEN_INFILE:
            return parse_redirect_input(data, current_command);
        case TOKEN_SEPARATOR:
            return parse_separator(data, current_index_of_command,
                                   current_index_of_arguments);
        case TOKEN_PIPELINE:
            return parse_concat(data, command_line, current_index_of_command,
                                current_index_of_arguments);
        default:
            return parse_add_command(data, current_command,
                                     current_index_of_command,
                                     current_index_of_arguments,
                                     number_of_command);
    }
}

static char *blank_skip(char *data) {
    while (isspace(*data) && !is_end(data)) {
        ++data;
    }

    return data;
}

static int parse_concat(char **data,
                        CommandLine *command_line,
                        size_t *current_index_of_command,
                        size_t *current_index_of_arguments) {
    if (*current_index_of_arguments == 0
        || *current_index_of_command + 1 == MAX_COMMANDS) {
        PRINT_SYNTAX_ERROR(TOKEN_PIPELINE_STR);
        return BAD_SYNTAX;
    }

    command_line->commands[(*current_index_of_command)++].flag |= OUT_PIPE;
    command_line->commands[*current_index_of_command].flag |= IN_PIPE;

    set_end(data);
    *current_index_of_arguments = 0;
    return SUCCESS;
}

static int parse_add_command(char **data,
                             Command *current_command,
                             const size_t *current_index_of_command,
                             size_t *current_index_of_arguments,
                             size_t *number_of_command) {
    if (*current_index_of_arguments == 0) {
        *number_of_command = *current_index_of_command + 1;
    }

    if (*current_index_of_arguments + 1 == MAX_ARGS) {
        fprintf(stderr, "shell: number of arguments (%d) exceeded\n", MAX_ARGS);
        return BAD_SYNTAX;
    }

    current_command->arguments[(*current_index_of_arguments)++] = *data;
    current_command->arguments[(*current_index_of_arguments)] = (char *) NULL;

    go_to_next_delimiter(data);
    return SUCCESS;
}

static int parse_separator(char **data,
                           size_t *current_index_of_command,
                           size_t *current_index_of_arguments) {
    if (*current_index_of_arguments == 0) {
        PRINT_SYNTAX_ERROR(TOKEN_SEPARATOR_STR);
        return BAD_SYNTAX;
    }

    set_end(data);
    ++(*current_index_of_command);
    *current_index_of_arguments = 0;
    return SUCCESS;
}

static int parse_background(char **data,
                            CommandLine *command_line,
                            size_t *current_index_of_command,
                            size_t *current_index_of_arguments) {
    if (*current_index_of_arguments == 0) {
        PRINT_SYNTAX_ERROR(TOKEN_BACKGROUND_STR);
        return BAD_SYNTAX;
    }

    command_line->commands[(*current_index_of_command)++].flag |= BACKGROUND;
    *current_index_of_arguments = 0;
    set_end(data);
    return SUCCESS;
}

static int parse_redirect_output(char **data, Command *command) {
    if ((*data)[1] == TOKEN_OUTFILE) {
        command->appfile = TRUE;
        set_end(data);
    } else {
        command->appfile = FALSE;
    }

    set_end(data);
    *data = blank_skip(*data);
    if (is_end(*data)) {
        PRINT_SYNTAX_ERROR(TOKEN_OUTFILE_STR);
        return BAD_SYNTAX;
    }

    command->outfile = *data;
    command->flag |= OUT_FILE;

    go_to_next_delimiter(data);
    return SUCCESS;
}

static int parse_redirect_input(char **data, Command *command) {
    set_end(data);
    *data = blank_skip(*data);
    if (is_end(*data)) {
        PRINT_SYNTAX_ERROR(TOKEN_INFILE_STR);
        return BAD_SYNTAX;
    }

    command->infile = *data;
    command->flag |= IN_FILE;

    go_to_next_delimiter(data);
    return SUCCESS;
}

static void go_to_next_delimiter(char **data) {
    *data = strpbrk(*data, delimiters);
    if (isspace(**data) && !is_end(*data)) {
        set_end(data);
    }
}

static void set_end(char **data) {
    **data = END;
    ++*data;
}

static void reset_command_line(CommandLine *command_line) {
    memset(command_line->commands, 0, sizeof(Command) * MAX_COMMANDS);
}
