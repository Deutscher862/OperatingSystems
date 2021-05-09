#include <semaphore.h>

#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PIZZA_AMOUNT 5
#define N 3 //number of cooks
#define M 3 //number of deliverers
#define oven_name "/OVEN"
#define table_name "/TABLE"

typedef struct{
    int values[MAX_PIZZA_AMOUNT];
} pizza_memory;

extern const char* semaphors_names[];
sem_t* semaphors[4];

void errorMessage(char* message);
int semValue(int index);

#endif