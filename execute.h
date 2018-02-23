#ifndef EXECUTE_H
#define EXECUTE_H


#include "command.h"


#define EXIT 1
#define CONTINUE 0
#define CRASH (-1)
#define STOP 2

#define BAD_PID (-1)
#define DESCENDANT_PID 0


int execute_command_line(CommandLine *command_line, int number_of_commands);


#endif //EXECUTE_H
