#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H


#include "command.h"


struct Job_St {
    int jid;
    pid_t pid;
    Command *command;
    char flag;
    char *infile;
    char *outfile;
    char appfile;
};

typedef struct Job_St Job;


#endif //JOB_CONTROL_H
