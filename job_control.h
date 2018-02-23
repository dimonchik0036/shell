/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H


#include "command.h"
#include "job.h"


#define JOB_LIMIT 32


struct JobController_St {
    Job *jobs[JOB_LIMIT];
    jid_t current_max_jid;
    int number_of_jobs;
};

typedef struct JobController_St JobController;


JobController *job_controller_create();

void job_controller_free(JobController *controller);

void job_controller_init(JobController *controller);

int job_controller_release(JobController *controller);

int job_controller_add_job(JobController *controller,
                           pid_t pid,
                           Command const *command,
                           char status);

int job_controller_remove_job(JobController *controller, jid_t jid);


#endif //JOB_CONTROL_H
