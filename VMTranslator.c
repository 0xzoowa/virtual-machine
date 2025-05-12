#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <limits.h>
#include "hack-implementation/hack.h"
#include "parser/parser.h"
#include "helper.h"

void process_command(const char *);

char out_file[MAX_CHAR];
Command cmd_type;
const char *cmd_str;
const char *arg1;
int arg2;
char path[PATH_MAX];

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments, please provide a filename or path.");
        return 1;
    }

    const char *input = argv[1];

    // construct output file with the specified extension
    const char *filename = remove_extension(input);
    snprintf(out_file, sizeof(out_file), "%s.asm", filename);

    DIR *dir = opendir(input);
    if (dir == NULL) // if null, should be a file
    {
        set_file_name(get_filename_without_extension(input));
        process_command(input);
    }
    else
    {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            // skip .(current dir) and ..(parent dir)
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(path, sizeof(path), "%s/%s", input, entry->d_name);
            // set_file_name
            set_file_name(remove_extension(entry->d_name));
            process_command(path);
        }
        closedir(dir);
    }
    end();
    platform_destroy();
    parser_destroy();

    return 0;
}

void process_command(const char *file)
{
    bool is_valid_file = validate_input_file(file);
    if (!is_valid_file)
    {
        exit(EXIT_FAILURE);
    }

    parser_create(file);
    platform_create(out_file);

    while (has_more_lines())
    {
        advance();
        init_props();
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
}
