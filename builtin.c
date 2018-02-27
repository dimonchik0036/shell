/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "builtin.h"
#include "execute.h"
#include "terminal.h"

#include <signal.h>
#include <wait.h>


static int builtin_cd(Command *command);

static int builtin_jobs(JobController *controller, Command *command);

static int builtin_fg(JobController *controller, Command *command);

static int builtin_bg(JobController *controller, Command *command);

static int builtin_exit(JobController *controller);


int builtin_exec(JobController *controller, Command *command) {
    char *command_name = command->arguments[0];
    if (!strcmp(command_name, "cd")) {
        return builtin_cd(command);
    } else if (!strcmp(command_name, "jobs")) {
        return builtin_jobs(controller, command);
    } else if (!strcmp(command_name, "fg")) {
        return builtin_fg(controller, command);
    } else if (!strcmp(command_name, "bg")) {
        return builtin_bg(controller, command);
    } else if (!strcmp(command_name, "exit")) {
        return builtin_exit(controller);
    }

    return CONTINUE;
}

static int builtin_cd(Command *command) {
    if (command->arguments[1] && command->arguments[2]) {
        fprintf(stderr, "cd: too many arguments");
        return CRASH;
    } else if (chdir(command->arguments[1]) == BAD_RESULT) {
        perror("cd");
        return CRASH;
    }

    return STOP;
}

static int builtin_jobs(JobController *controller, Command *command) {
    job_controller_print_all_jobs(controller);
    return STOP;
}

static int builtin_exit(JobController *controller) {
    job_controller_release(controller);
    return EXIT;
}

static int builtin_fg(JobController *controller, Command *command) {
    if (!controller->number_of_jobs) {
        return STOP;
    }

    size_t job_index = (size_t) (controller->number_of_jobs - 1);
    Job *job = controller->jobs[job_index];


    if (terminal_set_stdin(job->pid) == BAD_RESULT) {
        return CRASH;
    }

    job_kill(job, SIGCONT);
    int status;
    if (waitpid(job->pid, &status, WUNTRACED) != BAD_RESULT) {
        if (WIFSTOPPED(status)) {
            job->status = JOB_STOPPED;
            job_print(job, stdout, "");
        } else {
            job->status = JOB_DONE;
        }
    } else {
        perror("Couldn't wait for child process termination");
    }

    if (terminal_set_stdin(getpgrp()) == BAD_RESULT) {
        return CRASH;
    }

    if (job->status & JOB_DONE) {
        fprintf(stdout, "%s\n", command_get_name(job->command));
        fflush(stdout);
        job_controller_remove_job_by_index(controller, job_index);
    }

    return STOP;
}

static int builtin_bg(JobController *controller, Command *command) {
    if (!controller->number_of_jobs) {
        return STOP;
    }

    Job *last_job = controller->jobs[controller->number_of_jobs - 1];
    job_kill(last_job, SIGCONT);
    last_job->status = JOB_RUNNING;
    job_print(last_job, stdout, "");
    return STOP;
}
