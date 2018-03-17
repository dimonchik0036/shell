/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include <signal.h>
#include <wait.h>
#include "job.h"


Job *job_create(jid_t jid, pid_t pid, Command *command, char status) {
    Job *job = malloc(sizeof(Job));
    check_memory(job);

    job->command = command;
    job->status = status;
    job->jid = jid;
    job->pid = pid;
    return job;
}

void job_free(Job *job) {
    if (!job) {
        return;
    }

    command_free(job->command);
    free(job);
}

void job_swap(Job **lhs, Job **rhs) {
    Job *tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

void job_kill(Job *job, int signal) {
    kill(job->pid, signal);
}

void job_killpg(Job *job, int signal) {
    killpg(job->pid, signal);
}

char *job_get_status(char status) {
    switch (status) {
        case JOB_STOPPED:
            return "Stopped";
        case JOB_RUNNING:
            return "Running";
        case JOB_DONE:
            return "Done";
        case JOB_FAILED:
            return "Failed";
        default:
            return "Unexpected";
    }
}

void job_print(Job *job, FILE *file, char *prefix) {
    if (job->status & JOB_RUNNING) {
        fprintf(file, "%s[%d] %s %s %s &\n",
                prefix,
                job->jid, job_get_status(job->status),
                command_get_name(job->command),
                command_get_args(job->command));
    } else {
        fprintf(file, "%s[%d] %s %s %s\n",
                prefix,
                job->jid, job_get_status(job->status),
                command_get_name(job->command),
                command_get_args(job->command));
    }
}

void job_wait(Job *job) {
    int status;
    pid_t wait_result = waitpid(job->pid, &status, WUNTRACED);
    if (wait_result != BAD_RESULT) {
        if (WIFSTOPPED(status)) {
            job->status = JOB_STOPPED;
            job_print(job, stdout, "");
        } else {
            job->status = JOB_DONE;
        }
    } else {
        perror("Couldn't wait for child process termination");
    }
}

void job_set_status(Job *job, int status) {
    if (WIFEXITED(status)) {
        if (WEXITSTATUS(status) != EXIT_SUCCESS) {
            job->status = JOB_FAILED;
        } else {
            job->status = JOB_DONE;
        }
    } else if (WIFSTOPPED(status)) {
        job->status = JOB_STOPPED;
    } else if (WIFCONTINUED(status)) {
        job->status = JOB_RUNNING;
    }
}
