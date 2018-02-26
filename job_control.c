/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include <signal.h>
#include <wait.h>
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

jid_t job_controller_add_job(JobController *controller,
                             pid_t pid,
                             Command const *command,
                             char status) {
    if (controller->number_of_jobs >= JOB_LIMIT - 1) {
        fprintf(stderr, "shell: number of jobs (%d) exceeded\n", JOB_LIMIT);
        return BAD_RESULT;
    }

    Command *copy_of_command = command_copy(command);
    Job *job = job_create(controller->current_max_jid++, pid, copy_of_command, status);
    controller->jobs[controller->number_of_jobs++] = job;
    return job->jid;
}

int job_controller_remove_job_by_jid(JobController *controller, jid_t jid) {
    //TODO do it
    return EXIT_FAILURE;
}

int job_controller_remove_job_by_pid(JobController *controller, pid_t pid) {
    //TODO do it
    return EXIT_FAILURE;
}

int job_controller_remove_job_by_index(JobController *controller, size_t index) {
    Job *current_job = controller->jobs[index];

    size_t last_index = (size_t) (controller->number_of_jobs - 1);
    memmove(controller->jobs + index, controller->jobs + index + 1,
            (last_index - index) * sizeof(Job *));
    --controller->number_of_jobs;
    controller->jobs[controller->number_of_jobs] = NULL;

    if (!controller->number_of_jobs) {
        controller->current_max_jid = 1;
    }

    job_free(current_job);
    return EXIT_SUCCESS;
}

int job_controller_release(JobController *controller) {
    size_t index;
    for (index = 0; index < controller->number_of_jobs; ++index) {
        Job *current_job = controller->jobs[index];
        kill(current_job->pid, SIGHUP);
    }

    return EXIT_SUCCESS;
}

void job_controller_print_all_jobs(JobController *controller) {
    size_t index;
    for (index = 0; index < controller->number_of_jobs; ++index) {
        Job *current_job = controller->jobs[index];
        printf("[%d] %s %s\n", current_job->jid, job_get_status(current_job->status),
               command_get_string(current_job->command));
    }
}

void job_controller_print_current_status(JobController *controller) {
    size_t index;
    for (index = 0; index < controller->number_of_jobs; ++index) {
        Job *current_job = controller->jobs[index];
        int status;
        pid_t answer = waitpid(-current_job->pid, &status, WNOHANG | WUNTRACED);
        if (answer == BAD_RESULT) {
            perror("shell: job controller");
        } else if (!answer) {
        } else if (WIFEXITED(status)) {
            current_job->status = JOB_DONE;
            printf("[%d] %s %s\n", current_job->jid, job_get_status(current_job->status),
                   command_get_string(current_job->command));
            job_controller_remove_job_by_index(controller, index);
            --index;
        } else if (WIFSTOPPED(status)) {
            current_job->status = JOB_STOPPED;
            printf("[%d] %s %s\n", current_job->jid, job_get_status(current_job->status),
                   command_get_string(current_job->command));
        }
    }
}
