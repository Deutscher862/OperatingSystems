#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

const char* semaphors_names[] = { "/OVENACCESS", "/OVENPIZZAS", "/TABLEACCESS", "/TABLEPIZZAS"};

void errorMessage(char* message){
    printf("Error: %s\n", message);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int semValue(int index) {
    int value;
    sem_getvalue(semaphors[index], &value);
    return value;
}