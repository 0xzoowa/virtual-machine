#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hack.h"
#include "../parser/parser.h"
#include "../helper.h"

static char *get_code(const char *, int index);
static void hack_push(int, const char *);
static void hack_pop(int, const char *);
void set_file_name(char *filename);
static void vm_command();
static void initialize_lcl(int n_vars);
static void bootstrap_code();

static FILE *output_file = NULL;
static char *basename = NULL;
static char static_segment[MAX_CHAR];
static char *current_function_name = NULL;
static long return_label_counter = 0;

void platform_create(char *filename)
{

    output_file = fopen(filename, "w");
    if (output_file == NULL)
    {
        fprintf(stderr, "Error: Could not create output file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    bootstrap_code();
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

void write_arithmetic(const char *command)
{
    if (command == NULL)
    {
        fprintf(stderr, "Invalid command");
        exit(EXIT_FAILURE);
    }
    static int label_counter = 0; // for generating unique labels

    vm_command();
    // Unary operations
    // if (strcmp(command, "neg") == 0 || strcmp(command, "not") == 0) // pop y
    // {
    //     fprintf(output_file, "@SP\nAM=M-1\nD=M\n");
    //     if (strcmp(command, "neg") == 0)
    //     {
    //         fprintf(output_file, "D=-D\n"); // negate y
    //     }
    //     else
    //     {
    //         fprintf(output_file, "D=!D\n"); // not y
    //     }
    //     fprintf(output_file, "@SP\nA=M\nM=D\n@SP\nM=M+1\n"); // push y, increment sp
    //     return;
    // }
    if (strcmp(command, "neg") == 0)
    {
        fprintf(output_file,
                "@SP\n"
                "A=M-1\n"
                "M=-M\n");
        return;
    }
    else if (strcmp(command, "not") == 0)
    {
        fprintf(output_file,
                "@SP\n"
                "A=M-1\n"
                "M=!M\n");
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
                "@R13\nD=D-M\n"    // x - y
                "@TRUE_%d\nD;%s\n" // if condition met jump to TRUE
                "@SP\nA=M\nM=0\n"  // false case
                "@CONTINUE_%d\n0;JMP\n"
                "(TRUE_%d)\n"
                "@SP\nA=M\nM=-1\n" // true case
                "(CONTINUE_%d)\n"
                "@SP\nM=M+1\n", // increment SP
                id, jump, id, id, id);
        return;
    }
    return;
}

void write_push_pop(Command command, const char *segment, int index)
{
    // Check if segment is NULL or empty
    if (!segment || segment[0] == '\0')
    {
        fprintf(stderr, "Segment cannot be NULL or empty\n");
        exit(EXIT_FAILURE);
    }

    // Check if index is negative
    if (index < 0)
    {
        fprintf(stderr, "Index cannot be negative\n");
        exit(EXIT_FAILURE);
    }

    vm_command();
    switch (command)
    {
    case C_PUSH:
        if (strcmp(segment, "constant") == 0)
        {
            fprintf(output_file, "@%d\n"
                                 "D=A\n"
                                 "@SP\n"
                                 "A=M\n"
                                 "M=D\n"
                                 "@SP\n"
                                 "M=M+1\n",
                    index);
            return;
        }
        else if (strcmp(segment, "pointer") == 0)
        {
            if (index < 0 || index > 1)
            {
                fprintf(stderr, "Error: pointer index %d out of range (0-1)\n", index);
                return;
            }
            char *code = get_code(segment, index);

            // push value of this into stack
            fprintf(output_file, "@%s\n"
                                 "D=M\n"
                                 "@SP\n"
                                 "A=M\n"
                                 "M=D\n"
                                 "@SP\n"
                                 "M=M+1\n",
                    code);

            // if (index == 0)
            // {
            //     char *code = get_code(segment, index);
            //     // push value of this into stack
            //     fprintf(output_file, "@%s\n"
            //                          "D=M\n"
            //                          "@SP\n"
            //                          "A=M\n"
            //                          "M=D\n"
            //                          "@SP\n"
            //                          "M=M+1\n",
            //             code);
            // }
            // else if (index == 1)
            // {
            //     char *code = get_code(segment, index);
            //     // push value of that into the stack
            //     fprintf(output_file, "@%s\n"
            //                          "D=M\n"
            //                          "@SP\n"
            //                          "A=M\n"
            //                          "M=D\n"
            //                          "@SP\n"
            //                          "M=M+1\n",
            //             code);
            // }
            // else
            // {
            //     fprintf(stderr, "Error: Invalid pointer index %d (must be 0 or 1)\n", index);
            // }

            return;
        }
        else if (strcmp(segment, "temp") == 0)
        {
            if (index < 0 || index > 7)
            {
                fprintf(stderr, "Error: temp index %d out of range (0-7)\n", index);
                return;
            }
            char *code = get_code(segment, index);
            fprintf(output_file, "@%s\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", code);
            return;
        }
        else if (strcmp(segment, "static") == 0)
        {
            char *code = get_code(segment, index);
            snprintf(static_segment, sizeof(static_segment), "%s.%d", basename, index);
            // fprintf(output_file, "@%s\n"
            //                      "D=A\n"
            //                      "@%i\n"
            //                      "A=D+A\n"
            //                      "D=M\n"
            //                      "@SP\n"
            //                      "A=M\n"
            //                      "M=D\n"
            //                      "@SP\n"
            //                      "M=M+1\n",
            //         code, index);
            fprintf(output_file, "@%s\n"
                                 "D=M\n"
                                 "@SP\n"
                                 "A=M\n"
                                 "M=D\n"
                                 "@SP\n"
                                 "M=M+1\n",
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
            if (index < 0 || index > 1)
            {
                fprintf(stderr, "Error: pointer index %d out of range (0-1)\n", index);
                return;
            }

            char *code = get_code(segment, index);

            // pop value from stack into this
            fprintf(output_file, "@SP\n"
                                 "AM=M-1\n"
                                 "D=M\n"
                                 "@%s\n"
                                 "M=D\n",
                    code);

            // if (index == 0)
            // {
            //     char *code = get_code(segment, index);

            //     // pop value from stack into this
            //     fprintf(output_file, "@SP\n"
            //                          "AM=M-1\n"
            //                          "D=M\n"
            //                          "@%s\n"
            //                          "M=D\n",
            //             code);
            // }
            // else if (index == 1)
            // {
            //     char *code = get_code(segment, index);
            //     // pop value from stack into this
            //     fprintf(output_file, "@SP\n"
            //                          "AM=M-1\n"
            //                          "D=M\n"
            //                          "@%s\n"
            //                          "M=D\n",
            //             code);
            // }
            // else
            // {
            //     fprintf(stderr, "Error: Invalid pointer index %d (must be 0 or 1)\n", index);
            // }
            return;
        }

        else if (strcmp(segment, "temp") == 0)
        {
            if (index < 0 || index > 7)
            {
                fprintf(stderr, "Error: temp index %d out of range (0-7)\n", index);
                return;
            }
            char *code = get_code(segment, index);

            fprintf(output_file, "@SP\nAM=M-1\nD=M\n@%s\nM=D\n", code);
            return;
        }
        else if (strcmp(segment, "static") == 0)
        {
            char *code = get_code(segment, index);
            snprintf(static_segment, sizeof(static_segment), "%s.%d", basename, index);
            // fprintf(output_file, "@%s\n"
            //                      "D=A\n"
            //                      "@%i\n"
            //                      "D=D+A\n"
            //                      "@R13\n"
            //                      "M=D\n"
            //                      "@SP\n"
            //                      "AM=M-1\n"
            //                      "D=M\n"
            //                      "@R13\n"
            //                      "A=M\n"
            //                      "M=D\n",

            //         code, index);
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
        fprintf(stderr, "Error: Invalid command type\n");
        break;
    }
}
static void hack_push(int index, const char *segment)
{
    char *code = get_code(segment, index);
    if (code == NULL)
    {
        fprintf(stderr, "Error getting segment code");
        return;
    }
    fprintf(output_file, "@%d\n"
                         "D=A\n"
                         "@%s\n"
                         "A=D+M\n"
                         "D=M\n"
                         "@SP\n"
                         "A=M\n"
                         "M=D\n"
                         "@SP\n"
                         "M=M+1\n",
            index, code);
}
static void hack_pop(int index, const char *segment)
{
    char *code = get_code(segment, index);
    if (code == NULL)
    {
        fprintf(stderr, "Error getting segment code");
        return;
    }

    fprintf(output_file,
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@R13\n"
            "M=D\n"
            "@%d\n" // offset
            "D=A\n"
            "@%s\n"
            "D=D+M\n" // new_loc =  base + offset
            "@R14\n"
            "M=D\n" // stored new_loc
            "@R13\n"
            "D=M\n"
            "@R14\n"
            "A=M\n"
            "M=D\n",
            index, code);

    return;
}

static char *get_code(const char *segment, int index)
{
    static char code_buf[10];
    if (strcmp(segment, "local") == 0)
    {
        return "LCL";
    }
    if (strcmp(segment, "static") == 0)
    {
        return "16";
    }
    else if (strcmp(segment, "argument") == 0)
    {
        return "ARG";
    }
    else if (strcmp(segment, "this") == 0)
    {
        return "THIS";
    }
    else if (strcmp(segment, "that") == 0)
    {
        return "THAT";
    }
    else if (strcmp(segment, "pointer") == 0)
    {
        snprintf(code_buf, sizeof(code_buf), "R%i", 3 + index);
        return code_buf;
    }
    else if (strcmp(segment, "temp") == 0)
    {
        snprintf(code_buf, sizeof(code_buf), "R%i", 5 + index);
        return code_buf;
    }
    else
        return NULL;
}

void write_label(const char *label)
{
    vm_command();

    // fprintf(output_file, "(%s.%s$%s)\n", basename, current_function_name, label);
    fprintf(output_file, "(%s)\n", label);
    return;
}

void write_if(const char *label)
{
    vm_command();
    // fprintf(output_file, "@SP\n"
    //                      "AM=M-1\n"
    //                      "D=M\n"
    //                      "@%s.%s$%s\n"
    //                      "D;JNE\n",
    //         basename, current_function_name, label);
    fprintf(output_file, "@SP\n"
                         "AM=M-1\n"
                         "D=M\n"
                         "@%s\n"
                         "D;JNE\n",
            label);
    return;
}
void write_goto(const char *label)
{
    vm_command();
    // fprintf(output_file, "@%s.%s$%s\n"
    //                      "0;JMP\n",
    //         basename, current_function_name, label);

    fprintf(output_file, "@%s\n"
                         "0;JMP\n",
            label);
    return;
}

void write_function(const char *function_name, int n_vars)
{
    vm_command();
    if (current_function_name != NULL)
    {
        free(current_function_name);
    }
    current_function_name = strdup(function_name);

    // generate function label
    char function_label[MAX_CHAR];
    snprintf(function_label, sizeof(function_label), "%s", function_name);

    fprintf(output_file,

            // emit function label
            "(%s)\n", function_label);
    initialize_lcl(n_vars);
}
void write_call(const char *function_name, int n_args)
{
    vm_command();
    // generate return address label
    char return_addr[MAX_CHAR];

    snprintf(return_addr, sizeof(return_addr), "%s$ret.%li", current_function_name, return_label_counter++);
    fprintf(output_file,

            // save return address
            "@%s\n"
            "D=A\n"
            "@SP\n"
            "A=M\n"
            "M=D\n"
            "@SP\n"
            "M=M+1\n"

            // save LCL
            "@LCL\n"
            "D=M\n"
            "@SP\n"
            "A=M\n"
            "M=D\n"
            "@SP\n"
            "M=M+1\n"

            // save ARG
            "@ARG\n"
            "D=M\n"
            "@SP\n"
            "A=M\n"
            "M=D\n"
            "@SP\n"
            "M=M+1\n"

            // save THIS
            "@THIS\n"
            "D=M\n"
            "@SP\n"
            "A=M\n"
            "M=D\n"
            "@SP\n"
            "M=M+1\n"

            // save THAT
            "@THAT\n"
            "D=M\n"
            "@SP\n"
            "A=M\n"
            "M=D\n"
            "@SP\n"
            "M=M+1\n"

            // reposition ARG
            "@SP\n"
            "D=M\n"
            "@5\n"
            "D=D-A\n"
            "@%i\n"
            "D=D-A\n"
            "@ARG\n"
            "M=D\n"

            // reposition LCL
            "@SP\n"
            "D=M\n"
            "@LCL\n"
            "M=D\n"

            // goto called function
            "@%s\n"
            "0;JMP\n"

            // emit label
            "(%s)\n",
            return_addr,
            n_args, function_name, return_addr);
}

void write_return()
{
    vm_command();
    fprintf(output_file,
            // FRAME = LCL
            "@LCL\n"
            "D=M\n"
            "@R13\n"
            "M=D\n"

            "@R13\n"
            "D=M\n"
            "@5\n"
            "A=D-A\n"
            "D=M\n"
            "@R14\n"
            "M=D\n"

            // *ARG = pop()
            "@SP\n"
            "AM=M-1\n"
            "D=M\n"
            "@ARG\n"
            "A=M\n"
            "M=D\n"

            // SP = ARG + 1
            "@ARG\n"
            "D=M+1\n"
            "@SP\n"
            "M=D\n"

            // THAT = *(FRAME - 1)
            "@R13\n"
            "D=M\n"
            "@1\n"
            "A=D-A\n"
            "D=M\n"
            "@THAT\n"
            "M=D\n"

            // THIS = *(FRAME - 2)
            "@R13\n"
            "D=M\n"
            "@2\n"
            "A=D-A\n"
            "D=M\n"
            "@THIS\n"
            "M=D\n"

            // ARG = *(FRAME - 3)
            "@R13\n"
            "D=M\n"
            "@3\n"
            "A=D-A\n"
            "D=M\n"
            "@ARG\n"
            "M=D\n"

            // LCL = *(FRAME - 4)
            "@R13\n"
            "D=M\n"
            "@4\n"
            "A=D-A\n"
            "D=M\n"
            "@LCL\n"
            "M=D\n"

            // goto RET
            "@R14\n"
            "A=M\n"
            "0;JMP\n");
}
void end()
{
    fprintf(output_file,
            "(END)\n"
            "@END\n"
            "0;JMP");
}

void set_file_name(char *filename)
{
    basename = filename;
}

static void vm_command()
{
    char *line = current_command();
    if (*line != '\0')
    {
        fprintf(output_file, "//%s\n", line);
    }
}

static void initialize_lcl(int n_vars)
{

    int count = 0;
    while (count < n_vars) // n_vars=5 : 0✅ 1✅ 2✅ 3✅ 4✅ 5❌
    {
        fprintf(output_file, "@SP\n"
                             "A=M\n"
                             "M=0\n"
                             "@SP\n"
                             "M=M+1\n");
        count++; // 1 2 3 4 5
    }
}

static void bootstrap_code()
{

    if (current_function_name != NULL)
    {
        free(current_function_name);
    }
    current_function_name = strdup("Sys.init");

    fprintf(output_file,
            "@256\n"
            "D=A\n"
            "@SP\n"
            "M=D\n");

    // call Sys.init with 0 arguments
    write_call("Sys.init", 0);
}