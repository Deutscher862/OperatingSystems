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

int shared_memory_id;

typedef struct sembuf sembuf;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

int getPizzaFromTable(){
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    semop(semaphore_id, buff, 1);

    pizzas* pizzas_on_table = shmat(shared_memory_id, NULL, 0);
    int pizza_id = semctl(semaphore_id, 2, GETVAL, NULL) - 1 + MAX_PIZZA_AMOUNT;
    int pizza = pizzas_on_table->values[pizza_id];
    pizzas_on_table->values[pizza_id] = -1;
    shmdt(pizzas_on_table);

    printf("%d %ld) Pobieram pizze: %d. Liczba pizz na stole: %d.\n", getpid(), time(NULL), pizza, semctl(semaphore_id, 2, GETVAL, NULL)-1);

    sembuf* buff2 = (sembuf*) malloc(2*sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    buff2[1].sem_num = 2;
    buff2[1].sem_op = -1;
    buff2[1].sem_flg = 0;

    semop(semaphore_id, buff2, 2);

    return pizza;
}

int main(){
    srand(time(NULL));

    semaphore_id = getSemaphore(0);
    //table_semaphore_id = getSemaphore(1);

    shared_memory_id = getSharedMemory(1);
    //table_shared_memory_id = getSharedMemory(3);

    while (1)
    {   
        while(semctl(semaphore_id, 2, GETVAL, NULL) == 0) {};
        
        int pizza = getPizzaFromTable();

        usleep(getRandomTime(4000, 5000));

        printf("(%d %ld) Dostarczam pizze: %d.\n", getpid(), time(NULL), pizza);

        usleep(getRandomTime(4000, 5000));
    }
}