#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hack.h"
#include "../parser/parser.h"

static FILE *output_file = NULL;

static char *get_code(char *);
static void hack_push(int, const char *);
static void hack_pop(int, const char *);

void platform_create(char *filename)
{

    output_file = fopen(filename, "a");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error: Could not create output file %s\n", filename);
        exit(EXIT_FAILURE); // perror vs printing to stderr?
    }
}

void platform_destroy()
{
    if (output_file != NULL)
    {
        fclose(output_file);
        output_file = NULL;
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
    /**
     * push constant x => [push(cmdstr) ,constant(segment), x(index)] C_PUSH->type(command)
     * command_props->type(command), command_props->arg1(segment), command_props->arg2(index)
     */
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
        }
        else
        {
            hack_push(index, segment);
        }
        break;

    case C_POP:
        hack_pop(index, segment);
        break;

    default:
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

static char *get_code(char *segment)
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
