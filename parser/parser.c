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
    char *cmdstr; // command type
    char *arg1;   // segment
    int arg2;     // non negative integer

} Command_Props;

static FILE *input = NULL;
static char current_line[MAX_LINE_LENGTH];
static char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH];
static Command_Props *cmd = NULL;

void parser_create(char *filename)
{
    cmd = malloc(sizeof(Command_Props));
    if (!cmd)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memset(cmd, 0, sizeof(Command_Props));

    input = fopen(filename, "r");
    if (input == NULL)
    {
        free(cmd);
        cmd = NULL;
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }
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
    if (cmd)
    {
        free(cmd->cmdstr);
        free(cmd->arg1);
        free(cmd);
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

void command_type() // analyse default values for structs and arrays
{
    int no_of_cmds = 0;
    if (*current_line != '\0')
        no_of_cmds = split_line(current_line, tokens);

    if (no_of_cmds > 0)
    {
        cmd = cmd_type(tokens[0]);
        if (cmd->type != INVALID_TYPE && cmd->type != C_ARITHMETIC)
        {
            if (no_of_cmds >= 3)
            {
                cmd->arg1 = strdup(tokens[1]);
                cmd->arg2 = atoi(tokens[2]);
            }
        }
    }
}

static int split_line(const char *line, char tokens[][MAX_TOKEN_LENGTH])
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
            if (strcmp(line, "push") == 0)
            {
                cmd->type = C_PUSH;
            }
            else if (strcmp(line, "pop") == 0)
            {
                cmd->type = C_POP;
            }
            else
            {
                cmd->type = C_ARITHMETIC;
            }
            cmd->cmdstr = strdup(line); // Make a copy
            return cmd;
        }
    }
    cmd->type = INVALID_TYPE;
    cmd->cmdstr = strdup("INVALID");
    return cmd;
}

char *arg1()
// check for default type(INVALID_TYPE) when function is called: char *result = arg1(); if (result != INVALID_TYPE) ...
// should not be called if the type of the current command is a c_return

{
    switch (cmd->type)
    {
    case C_ARITHMETIC:
        return cmd->cmdstr;

    case C_PUSH:
    case C_POP:
        return cmd->arg1;

    default:
        return cmd->cmdstr;
    }
}

int arg2()
// this function will be called only if the current command is a c_push, c_pop, c_function, c_call
{
    if (!cmd)
        return -1; // Error value
    return cmd->arg2;
}

Command_Props *get_current_command()
{
    return cmd;
}

Command get_current_cmd_type(void)
{
    return cmd ? cmd->type : INVALID_TYPE;
}
const char *get_current_cmdstr(void)
{
    return cmd ? cmd->cmdstr : NULL;
}
const char *get_current_arg1(void)
{
    return cmd ? cmd->arg1 : NULL;
}
int get_current_arg2(void)
{
    return cmd ? cmd->arg2 : -1;
}