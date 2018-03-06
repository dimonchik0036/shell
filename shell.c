/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "prompt_line.h"
#include "job_control.h"
#include "parse_line.h"
#include "execute.h"


int main(int argc, char *argv[]) {
    return shell_run();
}

int shell_run() {
    CommandLine command_line;
    JobController *controller = job_controller_create();
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    char buffer[MAX_COMMAND_LINE];
    ssize_t number_of_read = prompt_line(buffer, sizeof(buffer));
    while (number_of_read > 0) {
        ssize_t number_of_commands = parse_input_line(buffer, &command_line);
        int exit_code = execute_command_line(controller, &command_line,
                                             number_of_commands);
        switch (exit_code) {
            case CONTINUE:
                break;
            case EXIT:
                return EXIT_SUCCESS;
            default:
                return EXIT_FAILURE;
        }

        job_controller_print_current_status(controller);
        number_of_read = prompt_line(buffer, sizeof(buffer));
    }

    if (number_of_read < 0) {
        perror("Couldn't read input");
        return EXIT_FAILURE;
    }

    job_controller_free(controller);
    return EXIT_SUCCESS;
}

void check_memory(void *src) {
    if (!src) {
        fprintf(stderr, "Couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }
}
