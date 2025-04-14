#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"

#define MAX_LINE_LENGTH 256

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

void advance()
{
    if (has_more_lines())
    {
        fgets(current_line, MAX_LINE_LENGTH, input);
        // strip comments and white spaces: “ ignores all white space and comments.”
    }
}

void parser_destro()
{

    if (input != NULL)
    {
        fclose(input);
        input = NULL;
    }
}

static bool is_ignorable(char *line)
{

    while (isspace(*line))
    {
        line++;
    }

    if (*line == '\0')
    {
        return true;
    }

    if (strncmp(line, "//", 2) == 0)
    {
        return true;
    }

    return false;
}