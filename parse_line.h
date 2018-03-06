/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#ifndef PARSE_LINE_H
#define PARSE_LINE_H


#include "command.h"


#define BAD_SYNTAX (-1)
#define SUCCESS 0

#define TOKEN_BACKGROUND '&'
#define TOKEN_BACKGROUND_STR "&"

#define TOKEN_INFILE '<'
#define TOKEN_INFILE_STR "<"

#define TOKEN_OUTFILE '>'
#define TOKEN_OUTFILE_STR ">"

#define TOKEN_CONCAT '|'
#define TOKEN_CONCAT_STR "|"

#define TOKEN_SEPARATOR ';'
#define TOKEN_SEPARATOR_STR ";"


ssize_t parse_input_line(char *input_data, CommandLine *command_line);


#endif //PARSE_LINE_H
