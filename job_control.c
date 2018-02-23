/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "job_control.h"


JobController *job_controller_create() {
    JobController *controller = malloc(sizeof(JobController));
    if (!controller) {
        fprintf(stderr, "Couldn't allocate memory\n");
        exit(EXIT_FAILURE);
    }

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