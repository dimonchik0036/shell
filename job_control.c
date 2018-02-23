/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "job_control.h"


JobController *job_controller_create() {
    JobController *controller = malloc(sizeof(JobController));
    check_memory(controller);

    job_controller_init(controller);
    return controller;
}

void job_controller_free(JobController *controller) {
    free(controller); //TODO add memory cleanup for Job[]
}

void job_controller_init(JobController *controller) {
    memset(controller->jobs, 0, sizeof(Job *) * JOB_LIMIT);
    controller->current_max_jid = 1;
    controller->number_of_jobs = 0;
}

int job_controller_add_job(JobController *controller,
                           pid_t pid,
                           Command const *command,
                           char status) {
    if (controller->number_of_jobs >= JOB_LIMIT - 1) {
        fprintf(stderr, "shell: number of jobs (%d) exceeded\n", JOB_LIMIT);
        return EXIT_FAILURE;
    }

    Command *copy_of_command = command_copy(command);
    Job *job = job_create(controller->current_max_jid++, pid, copy_of_command, status);
    controller->jobs[controller->number_of_jobs++] = job;
    return EXIT_SUCCESS;
}

int job_controller_remove_job(JobController *controller, jid_t jid) {
    //TODO do it
    return EXIT_FAILURE;
}

int job_controller_release(JobController *controller) {
    return EXIT_FAILURE;
}
