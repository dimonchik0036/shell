/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


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
            return "WTF?!";
    }
}
