/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef JOB_H
#define JOB_H


#include "command.h"


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


Job *job_create(jid_t jid, pid_t pid, Command *command, char status);

void job_free(Job *job);


#endif //JOB_H
