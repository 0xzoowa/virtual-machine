/**
 * store “ LCL, ARG, THIS, and THAT” Base addresses in header file - MACROS
 * Hack header file (interface) - hack implementation
 * x86 header file (interface) - x86 implementation
 * etc
 *
 * a single client will consume these interfaces
 *
 *
 * “The VM translator accepts a single command-line argument, as follows:
 * prompt> VMTranslator source
 * where source is a file name of the form ProgName.vm.
 * The file name may contain a file path. If no path is specified, the VM translator operates on the current folder.
 * The first character in the file name must be an uppercase letter, and the vm extension is mandatory.”
 *
 * => MODULES
 *  “ VMTranslator(MAIN), a Parser, and a CodeWriter”
 * “The Parser’s job is to make sense out of each VM command, that is, understand what the command seeks to do.
 * The CodeWriter’s job is to translate the understood VM command into assembly instructions that realize the desired operation on the specified (Hack) platform.
 * The VMTranslator drives the translation process.(main)”
 *
 * every module should have a create and destroy function (create serves as the constructor -: construct the object in memory; destory is to discard the object after use)
 *
 *
 * ==> The  Parser:
 * “This module handles the parsing of a single .vm file.
 *  The parser provides services for reading a VM command,
 *  unpacking the command into its various components,
 *  and providing convenient access to these components.
 *  In addition, the parser ignores all white space and comments.
 *  The parser is designed to handle all the VM commands”
 *
 * “For example, if the current command is push local 2,
 *  then calling arg1() and arg2() would return, respectively,
 * "local" and 2. If the current command is add, then calling arg1() would return "add",
 *  and arg2() would not be called”
 *
 * 
 * ==> The CodeWriter
 * “This module translates a parsed VM command into Hack assembly code.”
 * “For example, calling writePushPop (C_PUSH,"local",2) would result in
 *  generating assembly instructions that implement the VM command push local 2.
 *  Another example: Calling WriteArithmetic("add") would result in
 *  generating assembly instructions that pop the two topmost elements from the stack,
 *  add them up, and push the result onto the stack.”
 *
 * 
 * ==> The VMTranslator=
 * “This is the main program that drives the translation process,
 *  using the services of a Parser and a CodeWriter.
 *  The program gets the name of the input source file, say Prog.vm, from the command-line argument.
 *  It constructs a Parser for parsing the input file Prog.vm and creates an output file, Prog.asm,
 *  into which it will write the translated assembly instructions.
 *  The program then enters a loop that iterates through the VM commands in the input file.
 *  For each command, the program uses the Parser and the CodeWriter services 
 *  for parsing the command into its fields and then generating from them a sequence of assembly instructions.
 *  The instructions are written into the output Prog.asm file.”
 *
 * IMPORTANT:
 * “it is recommended to end each machine language program with an infinite loop.
 *  Therefore, consider writing a private routine that writes the infinite loop code in assembly.
 *  Call this routine once, when you are done translating all the VM commands.”


 *
 */

// push constant 10
// pop local 2

//HACK ASSEMBLY LANGUAGE

/**
 *
 * sp -> RAM[0] = 500
 *
 *  push constant 10
 *
 * @10
 * D = A
 *
 *  @sp
 *  A = M
 *  M = D
 *
 * @sp
 * M = M + 1
 *
 * LCL -> RAM[3] = 300
 *
 * LCL(segment) + 2(offset/index) => computed
 *
 * sp--
 * @sp
 * M = M - 1
 *
 * @sp
 * A = M
 * D = M
 *
 * @computed
 * M = D
 *
 * push constant x
 *
 * arg 1 => constant
 * arg 2 => x
 *
 *
 * Next, handle the segments local, argument, this, and that;
 * this -> current object
 * that -> current array
 *
 * push local 2
 * pop local 2
 *
 * local base address + offset(index)
 *
 *
 *
 */
/**
 * 
 * 
 *     switch (command)
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
        else if (strcmp(segment, "local") == 0)
        {
            fprintf(output_file, " @%d\n"
                                 "D = A\n"
                                 "@LCL\n"
                                 "A = M + D\n" // base + i
                                 "D = M\n"
                                 "@SP\n"
                                 "A = M\n"
                                 "M = D\n"
                                 "@SP\n"
                                 "M = M + 1\n",
                    index);
        }
        else if (strcmp(segment, "argument") == 0)
        {
            fprintf(output_file, " @%d\n"
                                 "D = A\n"
                                 "@ARG\n"
                                 "A = M + D\n"
                                 "D = M\n"
                                 "@SP\n"
                                 "A = M\n"
                                 "M = D\n"
                                 "@SP\n"
                                 "M = M + 1\n",
                    index);
        }
        else if (strcmp(segment, "this") == 0)
        {
            fprintf(output_file, " @%d\n"
                                 "D = A\n"
                                 "@THIS\n"
                                 "A = M + D\n"
                                 "D = M\n"
                                 "@SP\n"
                                 "A = M\n"
                                 "M = D\n"
                                 "@SP\n"
                                 "M = M + 1\n",
                    index);
        }
        else if (strcmp(segment, "that") == 0)
        {
            fprintf(output_file, " @%d\n"
                                 "D = A\n"
                                 "@THAT\n"
                                 "A = M + D\n"
                                 "D = M\n"
                                 "@SP\n"
                                 "A = M\n"
                                 "M = D\n"
                                 "@SP\n"
                                 "M = M + 1\n",
                    index);
        }
        break;

    case C_POP:
        if (strcmp(segment, "local") == 0)
        {
            fprintf(output_file,
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@R13\n"
                    "M=D\n" // Save value
                    "@%d\n"
                    "D=A\n"
                    "@LCL\n"
                    "D=M+D\n" // ARG + 3
                    "@R14\n"
                    "M=D\n" // Save address
                    "@R13\n"
                    "D=M\n" // Restore value
                    "@R14\n"
                    "A=M\n"
                    "M=D\n",
                    index); // *argument[3] = value
        }
        else if (strcmp(segment, "argument") == 0)
        {
            fprintf(output_file,
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@R13\n"
                    "M=D\n"
                    "@%d\n"
                    "D=A\n"
                    "@ARG\n"
                    "D=M+D\n"
                    "@R14\n"
                    "M=D\n"
                    "@R13\n"
                    "D=M\n"
                    "@R14\n"
                    "A=M\n"
                    "M=D\n",
                    index);
        }
        else if (strcmp(segment, "this") == 0)
        {
            fprintf(output_file,
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@R13\n"
                    "M=D\n"
                    "@%d\n"
                    "D=A\n"
                    "@THIS\n"
                    "D=M+D\n"
                    "@R14\n"
                    "M=D\n"
                    "@R13\n"
                    "D=M\n"
                    "@R14\n"
                    "A=M\n"
                    "M=D\n",
                    index);
        }
        else if (strcmp(segment, "that") == 0)
        {
            fprintf(output_file,
                    "@SP\n"
                    "AM=M-1\n"
                    "D=M\n"
                    "@R13\n"
                    "M=D\n"
                    "@%d\n"
                    "D=A\n"
                    "@THAT\n"
                    "D=M+D\n"
                    "@R14\n"
                    "M=D\n"
                    "@R13\n"
                    "D=M\n"
                    "@R14\n"
                    "A=M\n"
                    "M=D\n",
                    index);
        }
        break;

    default:
        break;
    }
 */
