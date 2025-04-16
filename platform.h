#include <stdio.h>

#ifndef PLATFORM_H
#define PLATFORM_H

void platfrom_create(FILE *file);
void write_arithmetic(const char *command);
void write_push_pop(int command, const char *segment, int index);
void platform_destroy(void);

#endif