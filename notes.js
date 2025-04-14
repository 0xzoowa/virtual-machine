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
 *
 *
 */
