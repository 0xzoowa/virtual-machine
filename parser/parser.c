#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 10
#define MAX_TOKEN_LENGTH 64

static void strip_comment(char *line);
static void trim(char *line);
static void strip(char *line);
int split_line(const char *line, char tokens[][MAX_TOKEN_LENGTH]);
static Command_Props *cmd_type(const char *line);

typedef struct cmd_props
{
    Command type;
    char *cmd;
    char *arg1;
    int arg2;

} Command_Props;

static FILE *input = NULL;
static char current_line[MAX_LINE_LENGTH];
static char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH];

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

Command_Props *command_type() // analyse default values for structs and arrays
{
    int no_of_cmds = split_line(current_line, tokens);

    if (*current_line != '\0')

    {
        Command_Props *cmd = cmd_type(tokens[0]);
        if (cmd->type != INVALID_TYPE || cmd->type != C_ARITHMETIC)
        {
            if (no_of_cmds >= 3)
            {
                cmd->arg1 = tokens[1];
                cmd->arg2 = atoi(tokens[2]);
            }
        }
        return cmd;
    }

    return NULL;
}

int split_line(const char *line, char tokens[][MAX_TOKEN_LENGTH])
{
    int count = 0;
    const char *start = line;

    while (*start && count < MAX_TOKENS)
    {
        while (*start && isspace((unsigned char)*start))
            start++; // skip spaces

        if (*start == '\0')
            break;

        const char *end = start;
        while (*end && !isspace((unsigned char)*end))
            end++; // find end of token

        size_t len = end - start;
        if (len >= MAX_TOKEN_LENGTH)
            len = MAX_TOKEN_LENGTH - 1;

        strncpy(tokens[count], start, len);
        tokens[count][len] = '\0'; // null-terminate
        count++;

        start = end;
    }

    return count;
}
static Command_Props *cmd_type(const char *line)
{

    static Command_Props cmd_props;
    char *arithmetic_logical_cmds[] = {
        "add",
        "sub",
        "neg",
        "eq",
        "gt",
        "lt",
        "and",
        "or",
        "not",
        "pop",
        "push",
    };

    size_t len = sizeof(arithmetic_logical_cmds) / sizeof(arithmetic_logical_cmds[0]);

    for (int i = 0; i < len; i++)
    {
        if (strcmp(line, arithmetic_logical_cmds[i]) == 0)
        {
            if (strcmp(arithmetic_logical_cmds[i], "push") != 0 || strcmp(arithmetic_logical_cmds[i], "pop") != 0)
            {
                cmd_props.type = C_ARITHMETIC;
            }

            else if (strcmp(arithmetic_logical_cmds[i], "push") == 0)
            {
                cmd_props.type = C_PUSH;
            }
            else if (strcmp(arithmetic_logical_cmds[i], "pop") == 0)
            {
                cmd_props.type = C_POP;
            }
            cmd_props.cmd = arithmetic_logical_cmds[i];

            return &cmd_props;
        }
    }
    cmd_props.type = INVALID_TYPE;
    cmd_props.cmd = "INVALID";

    return &cmd_props;
}

char *arg1() // check for NULL when function is called: char *result = arg1(); if (result != NULL) ...
{
    Command_Props *command = command_type();
    if (command->type == C_RETURN)
        return NULL; // might be unneccessary; make the check from the client instead
    switch (command->type)
    {
    case C_ARITHMETIC:
        return command->cmd;

    case C_PUSH:
    case C_POP:
        return command->arg1;

    default:
        return command->cmd;
    }
}

int arg2(void) // this function will be called only if the current command is a c_push, c_pop, c_function, c_call
{
    Command_Props *cmd = command_type();
    return cmd->arg2;
}
