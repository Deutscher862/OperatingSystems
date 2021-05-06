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

#include "config.h"

int semaphore_id;

int table_memory;

typedef struct sembuf sembuf;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

int getPizzaFromTable(){
    while(semctl(semaphore_id, 2, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(sizeof(sembuf));
    buff[0].sem_num = 2;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    semop(semaphore_id, buff, 1);

    pizza_memory* pizzas_on_table = shmat(table_memory, NULL, 0);

    int pizza_id = 0;
    while(pizzas_on_table->values[pizza_id] == -1)
        pizza_id++;

    int pizza = pizzas_on_table->values[pizza_id];
    pizzas_on_table->values[pizza_id] = -1;
    shmdt(pizzas_on_table);

    printf("(%d %ld) Pobieram pizze: %d. Liczba pizz na stole: %d.\n", getpid(), time(NULL), pizza, semctl(semaphore_id, 3, GETVAL, NULL)-1);

    sembuf* buff2 = (sembuf*) malloc(2*sizeof(sembuf));
    buff2[0].sem_num = 2;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    buff2[1].sem_num = 3;
    buff2[1].sem_op = -1;
    buff2[1].sem_flg = 0;

    semop(semaphore_id, buff2, 2);

    return pizza;
}

int main(){
    srand(time(NULL));

    semaphore_id = getSemaphore(0);

    table_memory = getSharedMemory(2);

    while (1)
    {   
        if(semctl(semaphore_id, 2, GETVAL, NULL) == 0 && semctl(semaphore_id, 3, GETVAL, NULL) > 0){
                    
            int pizza = getPizzaFromTable();

            usleep(getRandomTime(4000, 5000));

            printf("(%d %ld) Dostarczam pizze: %d.\n", getpid(), time(NULL), pizza);

            usleep(getRandomTime(4000, 5000));
        }

    }
}