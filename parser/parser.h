#include <stdbool.h>

#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 10
#define MAX_TOKEN_LENGTH 64

typedef enum commandType
{
    INVALID_TYPE = 0,
    C_PUSH = 1,
    C_POP = 2,
    C_LABEL = 3,
    C_GOTO = 4,
    C_IF = 5,
    C_FUNCTION = 6,
    C_RETURN = 7,
    C_CALL = 8,
    C_ARITHMETIC = 9
} Command;

typedef struct cmd_props Command_Props; // Forward declaration (incomplete type)

// Accessor functions
Command get_current_cmd_type(void);
const char *get_current_cmdstr(void);
const char *get_current_arg1(void);
int get_current_arg2(void);
void init_props(void);

void parser_create(const char *file);
bool has_more_lines(void);
void advance();
Command_Props *command_type(void);
const char *arg1(void);
int arg2(void);
void parser_destroy(void);
char *current_command();

#endif