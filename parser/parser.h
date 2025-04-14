#include <stdbool.h>

#ifndef PARSER_H
#define PARSER_H

typedef struct _IO_FILE FILE;

typedef enum commandType
{
    C_ARITHMETIC = 1,
    C_PUSH = 2,
    C_POP = 3,
    C_LABEL = 4,
    C_GOTO = 5,
    C_IF = 6,
    C_FUNCTION = 7,
    C_RETURN = 8,
    C_CALL = 9
} Command;

void parser_create(char *file);
bool has_more_lines(void);
void advance(void);
Command command_type(void);
char *arg1(void);
int arg2(void);
void parser_destroy(void);

#endif