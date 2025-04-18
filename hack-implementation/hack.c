#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hack.h"
#include "../parser/parser.h"

static int compute_address(char *, int);

static FILE *output_file = NULL;

void platform_create(char *filename)
{

    output_file = fopen(filename, "a");
    if (output_file == NULL)
    {
        perror("Error opening/creating output file");
        return;
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

static int compute_address(char *segment, int offset)
{
    int address = 0;

    if (strcmp(segment, "local") == 0)

        address = LCL + offset;

    else if (strcmp(segment, "argument") == 0)

        address = ARG + offset;

    else if (strcmp(segment, "this") == 0)

        address = THIS + offset;

    else if (strcmp(segment, "that") == 0)

        address = THAT + offset;

    return address;
}

void write_arithmetic(const char *command)
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
