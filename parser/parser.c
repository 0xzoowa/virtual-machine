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
static int split_line(const char *line, char tokens[][MAX_TOKEN_LENGTH]);
static void cmd_type(const char *line);

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

void parser_create(const char *filename)
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
        perror("Error: Cannot open file");
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

void advance()
{
    do
    {
        if (!fgets(current_line, MAX_LINE_LENGTH, input))
        {
            current_line[0] = '\0'; // end of file
            return;
        }
        strip(current_line);
    } while (*current_line == '\0');
    // should return an empty string ('\0') if string/line contains only whitespace and comment
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
        free(cmd);
        cmd = NULL;
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
    int no_of_cmds = 0;
    if (*current_line != '\0')
        no_of_cmds = split_line(current_line, tokens);

    if (no_of_cmds > 0)
    {
        cmd_type(tokens[0]);
        if (cmd->type != INVALID_TYPE && cmd->type != C_ARITHMETIC)
        {
            if (no_of_cmds == 2)
            {
                cmd->arg1 = tokens[1];
            }
            if (no_of_cmds >= 3)
            {
                cmd->arg1 = tokens[1];
                cmd->arg2 = atoi(tokens[2]);
            }
            return cmd;
        }
        return cmd;
    }
    return NULL;
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

static void cmd_type(const char *line)
{
    char *arithmetic_logical_cmds[] = {

        "pop",
        "push",
        "function",
        "call",
        "return",
        "goto",
        "if-goto",
        "label",
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

    size_t len = sizeof(arithmetic_logical_cmds) / sizeof(arithmetic_logical_cmds[0]);

    for (size_t i = 0; i < len; i++)
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
            else if (strcmp(line, "function") == 0)
            {
                cmd->type = C_FUNCTION;
            }
            else if (strcmp(line, "call") == 0)
            {
                cmd->type = C_CALL;
            }
            else if (strcmp(line, "return") == 0)
            {
                cmd->type = C_RETURN;
            }
            else if (strcmp(line, "label") == 0)
            {
                cmd->type = C_LABEL;
            }
            else if (strcmp(line, "goto") == 0)
            {
                cmd->type = C_GOTO;
            }
            else if (strcmp(line, "if-goto") == 0)
            {
                cmd->type = C_IF;
            }
            else
            {
                cmd->type = C_ARITHMETIC;
            }
            cmd->cmdstr = arithmetic_logical_cmds[i];
            return;
        }
    }
    cmd->type = INVALID_TYPE;
    cmd->cmdstr = "invalid";
    return;
}

char *arg1()
{
    switch (cmd->type)
    {
    case C_ARITHMETIC:
        return cmd->cmdstr;

    case C_PUSH:
    case C_POP:
    case C_LABEL:
    case C_GOTO:
    case C_IF:
    case C_FUNCTION:
    case C_CALL:
        return cmd->arg1;

    default:
        return cmd->cmdstr; // invalid
    }
}

int arg2()
// should only be called if the current command is a c_push, c_pop, c_function, c_call
{
    return cmd->arg2;
}

void init_props()
{
    command_type();
}

Command get_current_cmd_type(void)
{
    return cmd->type;
}
const char *get_current_cmdstr(void)
{
    return cmd->cmdstr;
}
const char *get_current_arg1(void)
{
    const char *arg_1 = arg1();
    return arg_1;
}
int get_current_arg2(void)
{
    int arg_2 = arg2();
    return arg_2;
}

char *current_command()
{
    return current_line;
}