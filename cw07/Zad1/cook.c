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

int oven_memory;
int table_memory;

typedef struct sembuf sembuf;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

int bakePizza(int pizzaType){
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(2*sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    buff[1].sem_num = 1;
    buff[1].sem_op = 1;
    buff[1].sem_flg = 0;

    semop(semaphore_id, buff, 2);

    pizza_memory* pizzas_in_oven = shmat(oven_memory, NULL, 0);

    int pizza_id = 0;
    while(pizzas_in_oven->values[pizza_id] != -1)
        pizza_id++;

    pizzas_in_oven->values[pizza_id] = pizzaType;
    shmdt(pizzas_in_oven);

    printf("(%d %ld) Dodałem pizze: %d. Liczba pizz w piecu: %d.\n", getpid(), time(NULL), pizzaType, semctl(semaphore_id, 1, GETVAL, NULL));

    sembuf* buff2 = (sembuf*) malloc(sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    semop(semaphore_id, buff2, 1);

    return pizza_id;
}

int getPizzaFromOven(int pizza_id){
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    semop(semaphore_id, buff, 1);

    pizza_memory* pizzas_in_oven = shmat(oven_memory, NULL, 0);
    int pizza = pizzas_in_oven->values[pizza_id];
    pizzas_in_oven->values[pizza_id] = -1;
    shmdt(pizzas_in_oven);

    sembuf* buff2 = (sembuf*) malloc(sizeof(sembuf));

    buff2[0].sem_num = 1;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    semop(semaphore_id, buff2, 1);

    return pizza;
}

void putPizzaOnTable(int pizzaType){
    while(semctl(semaphore_id, 2, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(2*sizeof(sembuf));
    buff[0].sem_num = 2;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    buff[1].sem_num = 3;
    buff[1].sem_op = 1;
    buff[1].sem_flg = 0;

    semop(semaphore_id, buff, 2);

    pizza_memory* pizzas_on_table = shmat(table_memory, NULL, 0);

    int pizza_id = 0;
    while(pizzas_on_table->values[pizza_id] != -1)
        pizza_id++;

    pizzas_on_table->values[pizza_id] = pizzaType;
    shmdt(pizzas_on_table);

    printf("(%d %ld) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), time(NULL), pizzaType, semctl(semaphore_id, 1, GETVAL, NULL), semctl(semaphore_id, 3, GETVAL, NULL));
    
    sembuf* buff2 = (sembuf*) malloc(2*sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    buff2[1].sem_num = 2;
    buff2[1].sem_op = -1;
    buff2[1].sem_flg = 0;

    semop(semaphore_id, buff2, 2);
}

int main(){
    srand(time(NULL));

    semaphore_id = getSemaphore(0);

    oven_memory = getSharedMemory(1);
    table_memory = getSharedMemory(2);

    while (1)
    {   
        int pizza_type = (rand() + getpid()) % 10;
        printf("(%d %ld) Przygotowuje pizze: %d.\n", getpid(), time(NULL), pizza_type);
        usleep(getRandomTime(1000, 2000));

        while(semctl(semaphore_id, 0, GETVAL, NULL) == 1 || semctl(semaphore_id, 1, GETVAL, NULL) == MAX_PIZZA_AMOUNT){usleep(100);}
            
            int pizza_id = bakePizza(pizza_type);

            usleep(getRandomTime(4000, 5000));

            pizza_type = getPizzaFromOven(pizza_id);

            while(semctl(semaphore_id, 2, GETVAL, NULL) == 1 || semctl(semaphore_id, 3, GETVAL, NULL) == MAX_PIZZA_AMOUNT) {usleep(100);};

            putPizzaOnTable(pizza_type);
    }
}