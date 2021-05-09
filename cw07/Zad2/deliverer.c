#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "config.h"

int table_descriptor;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

int getPizzaFromTable(){
    while(semValue(2) == 1){};

    sem_post(semaphors[2]);

    pizza_memory* pizzas_on_table = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, table_descriptor, 0);

    int pizza_id = 0;
    while(pizzas_on_table->values[pizza_id] == -1)
        pizza_id++;

    int pizza = pizzas_on_table->values[pizza_id];
    pizzas_on_table->values[pizza_id] = -1;
    munmap(pizzas_on_table, sizeof(pizza_memory));

    printf("(%d %ld) Pobieram pizze: %d. Liczba pizz na stole: %d.\n", getpid(), time(NULL), pizza, semValue(3)-1);

    sem_wait(semaphors[2]);
    sem_wait(semaphors[3]);

    return pizza;
}

int main(){
    srand(time(NULL));

    for(int i = 0; i < 4; i++)
        semaphors[i] = sem_open(semaphors_names[i], O_RDWR);

    table_descriptor = shm_open(table_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    while (1)
    {   
        while(semValue(2) == 1 || semValue(3) == 0) {};
                    
            int pizza = getPizzaFromTable();

            usleep(getRandomTime(4000, 5000));

            printf("(%d %ld) Dostarczam pizze: %d.\n", getpid(), time(NULL), pizza);

            usleep(getRandomTime(4000, 5000));

    }
}