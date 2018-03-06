/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "prompt_line.h"


ssize_t prompt_line(char *buffer, size_t buffer_size) {
    ssize_t number_of_write = write(STDOUT_FILENO, PROMPT_LINE,
                                    strlen(PROMPT_LINE));
    if (number_of_write < 0) {
        return BAD_RESULT;
    }

    ssize_t number_of_read = read(STDIN_FILENO, buffer, buffer_size - 1);
    if (number_of_read >= 0) {
        buffer[number_of_read] = END;
    }

    return number_of_read;
}
