/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef JOB_H
#define JOB_H


#include "command.h"


#define JOB_STOPPED 1
#define JOB_RUNNING 2
#define JOB_DONE 4
#define JOB_FAILED 8


typedef int jid_t;

struct Job_St {
    jid_t jid;
    pid_t pid;
    size_t count;
    Command *command;
    char status;
};

typedef struct Job_St Job;


Job *job_create(jid_t jid, pid_t pid, Command *command, char status);

Job *job_create_conveyor(jid_t jid, pid_t pid, Command *command, char status, size_t jobs_count);

void job_free(Job *job);

void job_set_status(Job *job, int status);

void job_swap(Job **lhs, Job **rhs);

char *job_get_status(char status);

void job_kill(Job *job, int signal);

void job_killpg(Job *job, int signal);

void job_print(Job *job, FILE *file, char *prefix);

void job_wait(Job *job);


#endif //JOB_H
