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

int terminal_set_parent() {
    pid_t ppid = getppid();
    pid_t pgid = getpgid(ppid);
    int exit_code = terminal_set_stdin(pgid);
    if (exit_code == BAD_RESULT) {
        return CRASH;
    }

    return exit_code;
}

static int terminal_set(int fd,
                        pid_t pgrp,
                        int sig,
                        void (*sig_handler_before)(int),
                        void (*sig_handler_after)(int)) {
    signal(sig, sig_handler_before);
    int exit_code = tcsetpgrp(fd, pgrp);
    if (exit_code == BAD_RESULT) {
        perror("Couldn't set terminal foreground process group");
        signal(sig, sig_handler_after);
        return exit_code;
    }

    signal(sig, sig_handler_after);
    return EXIT_SUCCESS;
}
