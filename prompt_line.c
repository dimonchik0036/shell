/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "prompt_line.h"


ssize_t prompt_line(char *line, size_t line_size) {
    if (write(STDOUT_FILENO, PROMPT_LINE, strlen(PROMPT_LINE)) < 0) {
        return BAD_RESULT;
    }

    ssize_t number_of_read = read(STDIN_FILENO, line, line_size - 1);
    if (number_of_read >= 0) {
        line[number_of_read] = '\0';
    }

    return number_of_read;
}
