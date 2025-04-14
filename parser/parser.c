#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define MAX_LINE_LENGTH 256

typedef struct cmd_props
{
    Command type;
    const char *type_string;

} Command_Props;

static FILE *input = NULL;
static char current_line[MAX_LINE_LENGTH];

void parser_create(char *filename)
{

    input = fopen(filename, "r");
    if (input == NULL)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // continue the parsing process
}

bool has_more_lines()
{

    int ch = fgetc(input);
    if (ch == EOF)
        return false;
    ungetc(ch, input);
    return true;
}

void advance() // check if current_line is empty whenever you call advance
{
    if (has_more_lines())
    {
        fgets(current_line, MAX_LINE_LENGTH, input);
        // strip comments and white spaces
        strip(current_line); // should return an empty string ('\0') is string/line contains only whitespace and comment
    }
}

void parser_destroy()
{

    if (input != NULL)
    {
        fclose(input);
        input = NULL;
    }
}
static void strip_comment(char *line)
{
    char *comment_start = strstr(line, "//"); // locates the first occurrence of a substring within a larger string and returns a pointer to the first character of the substring if found or NULL if not found.
    if (comment_start != NULL)
    {
        *comment_start = '\0';
    }
}

static void trim(char *line)
{
    // trim whitespace from start and end of string

    char *start = line;
    while (isspace(*start))
        start++;
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end))
        end--;
    *(end + 1) = '\0';
    if (start != line)
        memmove(line, start, strlen(start) + 1);
}

static void strip(char *line)
{
    trim(line);
    strip_comment(line);
}

Command_Props *command_type(void)
{

    if (current_line != NULL && *current_line != '\0')

    {
        Command_Props *cmd = cmd_type(current_line);
        return cmd;
    }
}

static Command_Props *cmd_type(const char *line)
{
    static Command_Props cmd_props;
    const char *arithmetic_cmds[] = {
        "add",
        "sub",
        "neg",
        "eq",
        "gt",
        "lt",
        "and",
        "or",
        "not",
    };

    size_t len = sizeof(arithmetic_cmds) / sizeof(arithmetic_cmds[0]);

    for (int i = 0; i < len; i++)
    {
        if (strcmp(line, arithmetic_cmds[i]) == 0)
        {
            cmd_props.type = C_ARITHMETIC;
            cmd_props.type_string = arithmetic_cmds[i];
            return &cmd_props;
        }
    }
    cmd_props.type = INVALID_TYPE;
    cmd_props.type_string = "INVALID";
    return &cmd_props;
}

char *arg1(void) // check for NULL when function is called: char *result = arg1(); if (result != NULL) ...
{
    Command_Props *cmd = command_type();
    if (cmd->type == C_RETURN)
        return NULL;
    switch (cmd->type)
    {
    case C_ARITHMETIC:
        return cmd->type_string;

    default:
        break;
    }
}
int arg2(void)
{
    Command_Props *cmd = command_type();
    // if its a push/pop operation parse the command from command props and return the second argument
    // switch (cmd->type)
    // {
    // case C_PUSH:
    //     break;
    // case C_POP:
    //     break;
    // case C_FUNCTION:
    //     break;
    // case C_CALL:
    //     break;
    // default:
    //     break;
    // }
}
