#include <stdio.h>
#include "parser/parser.h"

#ifndef PLATFORM_H
#define PLATFORM_H

#define MAX_CHAR 256

void platform_create(char *filename);
void write_arithmetic(const char *command);
void write_push_pop(Command command, const char *segment, int index);
void platform_destroy(void);
void end(void);

#endif