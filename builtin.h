/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef BUILTIN_H
#define BUILTIN_H


#include "job_control.h"


int builtin_exec(JobController *controller, Command *command);


#endif //BUILTIN_H
