#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"


int main(int argc, char *argv[]) {
    CommandLine command_line;

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    char line[MAX_COMMAND_LINE];
    ssize_t number_of_read;
    for (number_of_read = prompt_line(line, sizeof(line));
         number_of_read > 0;
         number_of_read = prompt_line(line, sizeof(line))) {
        int number_of_commands;
        if ((number_of_commands = parse_input_line(line, &command_line)) <= 0) {
            continue;
        }

        switch (execute_command_line(&command_line, number_of_commands)) {
            case CONTINUE:
                break;
            case EXIT:
                return EXIT_SUCCESS;
            default:
                return EXIT_FAILURE;
        }
    }

    if (number_of_read < 0) {
        perror("Couldn't read input");
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
