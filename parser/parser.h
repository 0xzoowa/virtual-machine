#include <stdbool.h>

#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 10
#define MAX_TOKEN_LENGTH 64

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
    C_CALL = 9,
    INVALID_TYPE = 10
} Command;

typedef struct cmd_props Command_Props; // Forward declaration (incomplete type)

// Accessor functions
Command get_current_cmd_type(void);
const char *get_current_cmdstr(void);
const char *get_current_arg1(void);
int get_current_arg2(void);

void parser_create(char *file);
bool has_more_lines(void);
void advance(FILE *f);
Command_Props *command_type(void);
const char *arg1(void);
int arg2(void);
void parser_destroy(void);

#endif