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

static int builtin_jkill(JobController *controller, Command *command);

static size_t job_get_index(JobController *controller, char *str);


int builtin_exec(JobController *controller, Command *command) {
    char *command_name = command_get_name(command);

    if (!strcmp(command_name, "cd")) {
        return builtin_cd(command);
    } else if (!strcmp(command_name, "jobs")) {
        return builtin_jobs(controller, command);
    } else if (!strcmp(command_name, "fg")) {
        return builtin_fg(controller, command);
    } else if (!strcmp(command_name, "bg")) {
        return builtin_bg(controller, command);
    } else if (!strcmp(command_name, "jkill")) {
        return builtin_jkill(controller, command);
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
        fprintf(stderr, "shell: fg: current: no such job\n");
        return CRASH;
    }

    size_t job_index = job_get_index(controller, command->arguments[1]);
    if (job_index >= controller->number_of_jobs) {
        fprintf(stderr, "shell: fg:  %s: no such job\n", command->arguments[1]);
        return CRASH;
    }

    Job *job = controller->jobs[job_index];
    if (terminal_set_stdin(job->pid) == BAD_RESULT) {
        return CRASH;
    }

    job_killpg(job, SIGCONT);
    job_wait(job);

    pid_t pgrp = getpgrp();
    int exit_code = terminal_set_stdin(pgrp);
    if (exit_code == BAD_RESULT) {
        return CRASH;
    }

    if (job->status & JOB_DONE) {
        fprintf(stdout, "%s\n", command_get_name(job->command));
        job_controller_remove_job_by_index(controller, job_index);
    }

    return STOP;
}

static int builtin_bg(JobController *controller, Command *command) {
    if (!controller->number_of_jobs) {
        fprintf(stderr, "shell: bg: current: no such job\n");
        return CRASH;
    }

    size_t job_index = job_get_index(controller, command->arguments[1]);
    if (job_index >= controller->number_of_jobs) {
        fprintf(stderr, "shell: bg:  %s: no such job\n", command->arguments[1]);
        return CRASH;
    }

    Job *job = controller->jobs[job_index];
    job_killpg(job, SIGCONT);
    job->status = JOB_RUNNING;
    job_print(job, stdout, "");
    return STOP;
}

static int builtin_jkill(JobController *controller, Command *command) {
    if (!controller->number_of_jobs) {
        fprintf(stderr, "shell: jkill: current: no such job\n");
        return CRASH;
    }

    size_t job_index = job_get_index(controller, command->arguments[1]);
    if (job_index >= controller->number_of_jobs) {
        fprintf(stderr, "shell: jkill:  %s: no such job\n",
                command->arguments[1]);
        return CRASH;
    }

    Job *job = controller->jobs[job_index];
    job_killpg(job, SIGKILL);
    fprintf(stdout, "Done\n");
    job_controller_remove_job_by_index(controller, job_index);
    return STOP;
}

static size_t job_get_index(JobController *controller, char *str) {
    size_t job_index = (size_t) (controller->number_of_jobs - 1);
    if (str) {
        int jid = atoi(str);
        if (jid) {
            job_index = job_controller_search_job_by_jid(controller, jid);
        }
    }

    return job_index;
}
