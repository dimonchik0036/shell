#ifndef PARSE_LINE_H
#define PARSE_LINE_H


#include "shell.h"


#define BAD_SYNTAX (-1)


int parse_input_line(char *line, CommandLine *command_line);


#endif //PARSE_LINE_H
