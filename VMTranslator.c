#include <stdio.h>
#include <stdlib.h>
#include "hack-implementation/hack.h"

int main(int argc, char *argv[])
{
    int x;
    char *y = "42";
    x = atoi(y);
    printf("hello world\n");
    printf("debugger\n");
    printf("configured\n");
    printf("successfully\n");
    printf("%d\n", x);

    return 0;
}
