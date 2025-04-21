#include <stdio.h>
#include <stdlib.h>
#include "hack-implementation/hack.h"
#include "parser/parser.h"

int main(int argc, char *argv[])
{
    // extract filename argv[1]
    // parser_create(filename or pathname);

    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments, please provide a filename or path.");
        return -1;
    }
    FILE *test = fopen("out.vm", "a");
    if (test == NULL)
    {
        fprintf(stderr, "Error opening out file.");
        return -1;
    }
    parser_create(argv[1]);
    while (has_more_lines())
    {
        advance(test);
    }

    return 0;
}
