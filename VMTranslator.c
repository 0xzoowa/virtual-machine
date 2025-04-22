#include <stdio.h>
#include <stdlib.h>
#include "hack-implementation/hack.h"
#include "parser/parser.h"
#include "helper.h"

int main(int argc, char *argv[])
{
    char out_file[MAX_CHAR];
    Command cmd_type;
    const char *cmd_str;
    const char *arg1;
    int arg2;

    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments, please provide a filename or path.");
        return 1;
    }

    const char *file_input = argv[1];
    validate_input_file(file_input);

    char *filename = get_filename_without_extension(file_input);
    snprintf(out_file, sizeof(out_file), "%s.asm", filename);

    parser_create(file_input);
    platform_create(out_file);

    while (has_more_lines())
    {
        advance();
        cmd_type = get_current_cmd_type();
        switch (cmd_type)
        {
        case C_ARITHMETIC:
            cmd_str = get_current_cmdstr();
            write_arithmetic(cmd_str);
            break;

        case C_POP:
        case C_PUSH:
            arg1 = get_current_arg1();
            arg2 = get_current_arg2();
            write_push_pop(cmd_type, arg1, arg2);
            break;

        case INVALID_TYPE:
            break;

        default:
            break;
        }
    }

    platform_destroy();
    parser_destroy();

    return 0;
}
