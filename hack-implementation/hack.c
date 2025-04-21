#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hack.h"
#include "../parser/parser.h"

#define MAX_CHAR 256

static char *get_code(const char *);
static void hack_push(int, const char *);
static void hack_pop(int, const char *);
static char *get_filename_without_extension(const char *filename);

static FILE *output_file = NULL;
static char *basename = NULL;
static char static_segment[MAX_CHAR];

void platform_create(char *filename)
{

    output_file = fopen(filename, "a");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error: Could not create output file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    basename = get_filename_without_extension(filename);
    if (!basename)
    {
        fclose(output_file);
        output_file = NULL;
        fprintf(stderr, "Error: Could not extract basename from output file %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void platform_destroy()
{
    if (output_file != NULL)
    {
        fclose(output_file);
        output_file = NULL;
    }
    if (basename != NULL)
    {
        free(basename);
        basename = NULL;
    }
}

void write_arithmetic(const char *command) // command_props->cmdstr: argument
{
    static int label_counter = 0; // for generating unique labels

    // Unary operations
    if (strcmp(command, "neg") == 0 || strcmp(command, "not") == 0) // pop y
    {
        fprintf(output_file, "@SP\nAM=M-1\nD=M\n");
        if (strcmp(command, "neg") == 0)
        {
            fprintf(output_file, "D=-D\n"); // negate y
        }
        else
        {
            fprintf(output_file, "D=!D\n"); // not y
        }
        fprintf(output_file, "@SP\nA=M\nM=D\n@SP\nM=M+1\n"); // push y, increment sp
        return;
    }

    // Binary operations - pop y, pop x
    fprintf(output_file, "@SP\nAM=M-1\nD=M\n@R13\nM=D\n"); // y -> R13
    fprintf(output_file, "@SP\nAM=M-1\nD=M\n");            // x in D

    if (strcmp(command, "add") == 0)
    {
        fprintf(output_file, "@R13\nD=D+M\n");
        fprintf(output_file, "@SP\nA=M\nM=D\n");
        fprintf(output_file, "@SP\nM=M+1\n");
        return;
    }
    else if (strcmp(command, "sub") == 0)
    {
        fprintf(output_file, "@R13\nD=D-M\n");
        fprintf(output_file, "@SP\nA=M\nM=D\n");
        fprintf(output_file, "@SP\nM=M+1\n");
        return;
    }
    else if (strcmp(command, "and") == 0)
    {
        fprintf(output_file, "@R13\nD=D&M\n");
        fprintf(output_file, "@SP\nA=M\nM=D\n");
        fprintf(output_file, "@SP\nM=M+1\n");
        return;
    }
    else if (strcmp(command, "or") == 0)
    {
        fprintf(output_file, "@R13\nD=D|M\n");
        fprintf(output_file, "@SP\nA=M\nM=D\n");
        fprintf(output_file, "@SP\nM=M+1\n");
        return;
    }
    else if (
        strcmp(command, "eq") == 0 ||
        strcmp(command, "gt") == 0 ||
        strcmp(command, "lt") == 0)
    {
        const char *jump;
        if (strcmp(command, "eq") == 0)
            jump = "JEQ";
        else if (strcmp(command, "gt") == 0)
            jump = "JGT";
        else
            jump = "JLT";

        // Comparison logic
        int id = label_counter++;
        fprintf(output_file,
                "@R13\nD=D-M\n"                       // x - y
                "@TRUE%d\nD;%s\n"                     // if x-y==0|>0|<0 jump
                "@SP\nA=M\nM=0\n@CONTINUE%d\n0;JMP\n" // false (0)
                "(TRUE%d)\n@SP\nA=M\nM=-1\n"          // true (-1)
                "(CONTINUE%d)\n"
                "@SP\nM=M+1\n",
                id, jump, id, id, id);
    }
}

void write_push_pop(Command command, const char *segment, int index)
{
    switch (command)
    {
    case C_PUSH:
        if (strcmp(segment, "constant") == 0)
        {
            fprintf(output_file, "@%d\n"
                                 "D = A\n"
                                 "@SP\n"
                                 "A = M\n"
                                 "M = D\n"
                                 "@SP\n"
                                 "M = M + 1\n",
                    index);
            return;
        }
        else if (strcmp(segment, "pointer") == 0)
        {
            if (index == 0)
            {
                // push value of this into stack
                fprintf(output_file, "@THIS"
                                     "A=M"
                                     "D=M"
                                     "@SP"
                                     "A=M"
                                     "M=D"
                                     "@SP"
                                     "M = M + 1");
            }
            else if (index == 1)
            {
                // push value of that into the stack
                fprintf(output_file, "@THAT"
                                     "A=M"
                                     "D=M"
                                     "@SP"
                                     "A=M"
                                     "M=D"
                                     "@SP"
                                     "M = M + 1");
            }
            else
            {
                fprintf(stderr, "Error: Invalid pointer index %d (must be 0 or 1)\n", index);
            }

            return;
        }
        else if (strcmp(segment, "temp") == 0)
        {
            if (index < 0 || index > 7)
            {
                fprintf(stderr, "Error: temp index %d out of range (0-7)\n", index);
                return;
            }
            fprintf(output_file, "@%d\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", 5 + index);
            return;
        }
        else if (strcmp(segment, "static") == 0)
        {
            if (index < 0 || index > 239)
            {
                fprintf(stderr, "Error: static index %d out of range (0-239)\n", index);
                return;
            }

            snprintf(static_segment, sizeof(static_segment), "%s.%d", basename, index);
            fprintf(output_file, "@%s\n"
                                 "D=M\n"
                                 "@SP\n"
                                 "A=M\n"
                                 "M=D\n"
                                 "@SP\n"
                                 "M=M+1",
                    static_segment);
            return;
        }

        else
        {
            hack_push(index, segment);
            return;
        }

    case C_POP:

        if (strcmp(segment, "pointer") == 0)
        {

            if (index == 0)
            {

                // pop value from stack into this
                fprintf(output_file, "@SP"
                                     "AM=M-1"
                                     "D=M"
                                     "@THIS"
                                     "A=M"
                                     "M=D");
            }
            else if (index == 1)
            {
                // pop value from stack into this
                fprintf(output_file, "@SP"
                                     "AM=M-1"
                                     "D=M"
                                     "@THAT"
                                     "A=M"
                                     "M=D");
            }
            else
            {
                fprintf(stderr, "Error: Invalid pointer index %d (must be 0 or 1)\n", index);
            }
            return;
        }

        else if (strcmp(segment, "temp") == 0)
        {
            if (index < 0 || index > 7)
            {
                fprintf(stderr, "Error: temp index %d out of range (0-7)\n", index);
                return;
            }

            fprintf(output_file, "@SP\nAM=M-1\nD=M\n@%d\nM=D\n", 5 + index);
            return;
        }
        else if (strcmp(segment, "static") == 0)
        {
            if (index < 0 || index > 239)
            {
                fprintf(stderr, "Error: static index %d out of range (0-239)\n", index);
                return;
            }

            snprintf(static_segment, sizeof(static_segment), "%s.%d", basename, index);
            fprintf(output_file, "@SP\n"
                                 "AM=M-1\n"
                                 "D=M\n"
                                 "@%s\n"
                                 "M=D\n",

                    static_segment);
            return;
        }
        else
        {
            hack_pop(index, segment);
            return;
        }

    default:
        fprintf(stderr, "Error: Unknown command type\n");
        break;
    }
}
static void hack_push(int index, const char *segment)
{
    char *code = get_code(segment);
    if (code == NULL)
    {
        fprintf(stderr, "ERROR GETTING SEGMENT CODE");
        return;
    }
    fprintf(output_file, " @%d\n"
                         "D = A\n"
                         "@%s\n"
                         "A = M + D\n"
                         "D = M\n"
                         "@SP\n"
                         "A = M\n"
                         "M = D\n"
                         "@SP\n"
                         "M = M + 1\n",
            index, code);
}
static void hack_pop(int index, const char *segment)
{
    char *code = get_code(segment);
    if (code == NULL)
    {
        fprintf(stderr, "ERROR GETTING SEGMENT CODE");
        return;
    }

    fprintf(output_file,
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@R13\n"
            "M=D\n"
            "@%d\n"
            "D=A\n"
            "@%s\n"
            "D=M+D\n"
            "@R14\n"
            "M=D\n"
            "@R13\n"
            "D=M\n"
            "@R14\n"
            "A=M\n"
            "M=D\n",
            index, code);
}

static char *get_code(const char *segment)
{
    char *code;
    if (strcmp(segment, "local") == 0)
    {
        code = "LCL";
    }
    else if (strcmp(segment, "argument") == 0)
    {
        code = "ARG";
    }
    else if (strcmp(segment, "this") == 0)
    {
        code = "THIS";
    }
    else if (strcmp(segment, "that") == 0)
    {
        code = "THAT";
    }
    else
        code = NULL;

    return code;
}

static char *get_filename_without_extension(const char *filename)
{
    char *result;

    const char *dot = strrchr(filename, '.');

    const char *slash = strrchr(filename, '/'); // unix
    if (!slash)
    {
        slash = strrchr(filename, '\\'); // windows
    }

    if (!dot || (slash && dot < slash))
    {
        return NULL;
    }

    size_t len = dot - filename; // dot index - filename index
    result = (char *)malloc(len + 1);
    if (result)
    {
        strncpy(result, filename, len);
        result[len] = '\0';

        return result;
    }
    return NULL;
}
