/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef SHELL_H
#define SHELL_H


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define TRUE 1
#define FALSE 0

#define BAD_RESULT (-1)


int shell_run();

void check_memory(void *src);


#endif //SHELL_H
