/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "terminal.h"

#include <signal.h>


static int terminal_set(int fd,
                        pid_t pgrp,
                        int sig,
                        void (*sig_handler_before)(int),
                        void (*sig_handler_after)(int));


int terminal_set_stdin(pid_t pgrp) {
    return terminal_set(STDIN_FILENO, pgrp, SIGTTOU, SIG_IGN, SIG_DFL);
}

static int terminal_set(int fd,
                        pid_t pgrp,
                        int sig,
                        void (*sig_handler_before)(int),
                        void (*sig_handler_after)(int)) {
    signal(sig, sig_handler_before);
    if (tcsetpgrp(fd, pgrp) == BAD_RESULT) {
        perror("Couldn't set terminal foreground process group");
        signal(sig, sig_handler_after);
        return BAD_RESULT;
    }

    signal(sig, sig_handler_after);
    return EXIT_SUCCESS;
}
