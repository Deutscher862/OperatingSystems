#ifndef CONFIG_H
#define CONFIG_H

#define MAX_PIZZA_AMOUNT 5
#define N 3 //number of cooks
#define M 3 //number of deliverers

typedef struct{
    int values[MAX_PIZZA_AMOUNT];
} pizza_memory;

void errorMessage(char* message);
int getSemaphore(int id);
int getSharedMemory(int id);

#endif