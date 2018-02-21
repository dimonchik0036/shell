#include "shell.h"


#define PRINT_SYNTAX_ERROR(token) fprintf(stderr, "shell: syntax error near unexpected token '%s'\n", token)


static char *blank_skip(char *string);

static int parse_redirect_output(char **string, Command *command);

static int parse_redirect_input(char **string, Command *command);

static void go_to_next_delimiter(char **string);

static void set_end(char **string);

static void reset_command_line(CommandLine *command_line);


static char delimiters[] = " \t&<>;\n";


int parse_input_line(char *line, CommandLine *command_line) {
    reset_command_line(command_line);
    int number_of_arguments = 0;
    int number_of_command = 0;
    int result_number_of_command = 0;
    char *string = line;


    while (*string) {
        if (number_of_command >= MAX_COMMANDS) {
            fprintf(stderr, "shell: number of commands (%d) exceeded\n", MAX_COMMANDS);
            return BAD_SYNTAX;
        }

        string = blank_skip(string);
        if (!(*string)) {
            break;
        }

        switch (*string) {
            case '&':
                if (!number_of_arguments) {
                    PRINT_SYNTAX_ERROR("&");
                    return BAD_SYNTAX;
                }

                command_line->commands[number_of_command++].flag |= BACKGROUND;
                set_end(&string);
                number_of_arguments = 0;
                break;
            case '>':
                if (parse_redirect_output(&string,
                                          &command_line->commands[number_of_command])) {
                    return BAD_SYNTAX;
                };
                break;
            case '<':
                if (parse_redirect_input(&string,
                                         &command_line->commands[number_of_command])) {
                    return BAD_SYNTAX;
                };
                break;
            case ';':
                if (!number_of_arguments) {
                    PRINT_SYNTAX_ERROR(";");
                    return BAD_SYNTAX;
                }

                set_end(&string);
                ++number_of_command;
                number_of_arguments = 0;
                break;
            case '|':
                if (!number_of_arguments || number_of_command + 1 == MAX_COMMANDS) {
                    PRINT_SYNTAX_ERROR("|");
                    return BAD_SYNTAX;
                }

                command_line->commands[number_of_command++].flag |= OUT_PIPE;
                command_line->commands[number_of_command].flag |= IN_PIPE;

                set_end(&string);
                number_of_arguments = 0;
                break;
            default:
                if (!number_of_arguments) {
                    result_number_of_command = number_of_command + 1;
                }

                if (number_of_arguments + 1 == MAX_ARGS) {
                    fprintf(stderr, "shell: number of arguments (%d) exceeded\n",
                            MAX_ARGS);
                    return BAD_SYNTAX;
                }

                command_line->commands[number_of_command].arguments[number_of_arguments++] = string;
                command_line->commands[number_of_command].arguments[number_of_arguments] = (char *) NULL;

                go_to_next_delimiter(&string);

                break;
        }
    }

    if (result_number_of_command > 0) {
        if (command_line->commands[result_number_of_command - 1].flag & OUT_PIPE) {
            PRINT_SYNTAX_ERROR("|");
            return BAD_SYNTAX;
        }
    }

    return result_number_of_command;
}

static char *blank_skip(char *string) {
    while (isspace(*string) && (*string)) {
        ++string;
    }

    return string;
}

static int parse_redirect_output(char **string, Command *command) {
    if ((*string)[1] == '>') {
        command->appfile = TRUE;
        set_end(string);
    } else {
        command->appfile = FALSE;
    }

    set_end(string);
    *string = blank_skip(*string);
    if (!**string) {
        PRINT_SYNTAX_ERROR(">");
        return BAD_SYNTAX;
    }

    command->outfile = *string;
    command->flag |= OUT_FILE;

    go_to_next_delimiter(string);
    return EXIT_SUCCESS;
}

static int parse_redirect_input(char **string, Command *command) {
    set_end(string);
    *string = blank_skip(*string);
    if (!**string) {
        PRINT_SYNTAX_ERROR("<");
        return BAD_SYNTAX;
    }

    command->infile = *string;
    command->flag |= IN_FILE;

    go_to_next_delimiter(string);
    return EXIT_SUCCESS;
}

static void go_to_next_delimiter(char **string) {
    *string = strpbrk(*string, delimiters);
    if (isspace(**string) && **string) {
        set_end(string);
    }
}

static void set_end(char **string) {
    **string = '\0';
    ++*string;
}

static void reset_command_line(CommandLine *command_line) {
    command_line->start_pipeline_index = 0;
    command_line->current_pipeline_index = 0;
    memset(command_line->commands, 0, sizeof(Command) * MAX_COMMANDS);
}
