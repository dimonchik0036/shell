/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H


#include "command.h"


#define JOB_LIMIT 32


#define STOPPED 1
#define RUNNING 2
#define DONE 4
#define FAILED 8


typedef int jid_t;

struct Job_St {
    jid_t jid;
    pid_t pid;
    Command *command;
    char status;
};

typedef struct Job_St Job;

struct JobController_St {
    Job *jobs[JOB_LIMIT];
    jid_t current_max_jid;
    int number_of_jobs;
};

typedef struct JobController_St JobController;


JobController *job_controller_create();

void job_controller_free(JobController *controller);

void job_controller_init(JobController *controller);

void job_controller_add_job(pid_t pid, Command *command);


#endif //JOB_CONTROL_H
