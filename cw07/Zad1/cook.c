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

void bakePizza(int pizzaType){
    /*
     * 0 - czy aktualnie ktos modyfikuje tablice (0 - wolne, 1 - zajete)
     * 1 - ilosc pizz w piecu
     * 2 - ilosc pizz na stole
    */
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(2*sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    buff[1].sem_num = 1;
    buff[1].sem_op = 1;
    buff[1].sem_flg = 0;

    semop(semaphore_id, buff, 2);

    pizzas* pizzas_in_oven = shmat(shared_memory_id, NULL, 0);

    int pizza_id = semctl(semaphore_id, 1, GETVAL, NULL) - 1;
    pizzas_in_oven->values[pizza_id] = pizzaType;
    printf("(%d %ld) Dodałem pizze: %d. Liczba pizz w piecu: %d.\n", getpid(), time(NULL), pizzaType, semctl(semaphore_id, 1, GETVAL, NULL));
    shmdt(pizzas_in_oven);

    sembuf* buff2 = (sembuf*) malloc(sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    semop(semaphore_id, buff2, 1);
}

int getPizzaFromOven(){
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    semop(semaphore_id, buff, 1);

    pizzas* pizzas_in_oven = shmat(shared_memory_id, NULL, 0);
    int pizza_id = semctl(semaphore_id, 1, GETVAL, NULL) - 1;
    int pizza = pizzas_in_oven->values[pizza_id];
    pizzas_in_oven->values[pizza_id] = -1;
    shmdt(pizzas_in_oven);

    sembuf* buff2 = (sembuf*) malloc(2*sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    buff2[1].sem_num = 1;
    buff2[1].sem_op = -1;
    buff2[1].sem_flg = 0;

    semop(semaphore_id, buff2, 2);

    return pizza;
}

void putPizzaOnTable(int pizzaType){
    while(semctl(semaphore_id, 0, GETVAL, NULL) == 1) {};

    sembuf* buff = (sembuf*) malloc(2*sizeof(sembuf));
    buff[0].sem_num = 0;
    buff[0].sem_op = 1;
    buff[0].sem_flg = 0;

    buff[1].sem_num = 2;
    buff[1].sem_op = 1;
    buff[1].sem_flg = 0;

    semop(semaphore_id, buff, 2);

    pizzas* pizzas_on_table = shmat(shared_memory_id, NULL, 0);

    int pizza_id = semctl(semaphore_id, 2, GETVAL, NULL) - 1 + MAX_PIZZA_AMOUNT;
    pizzas_on_table->values[pizza_id] = pizzaType;
    printf("%d %ld) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), time(NULL), pizzaType, semctl(semaphore_id, 1, GETVAL, NULL), semctl(semaphore_id, 2, GETVAL, NULL));
    shmdt(pizzas_on_table);

    sembuf* buff2 = (sembuf*) malloc(sizeof(sembuf));
    buff2[0].sem_num = 0;
    buff2[0].sem_op = -1;
    buff2[0].sem_flg = 0;

    semop(semaphore_id, buff2, 1);
}

int main(){
    srand(time(NULL));

    semaphore_id = getSemaphore(0);
    //table_semaphore_id = getSemaphore(1);

    shared_memory_id = getSharedMemory(1);
    //table_shared_memory_id = getSharedMemory(3);

    while (1)
    {   
        int pizza_type = rand() % 10;
        printf("(%d %ld) Przygotowuje pizze: %d.\n", getpid(), time(NULL), pizza_type);
        usleep(getRandomTime(1000, 2000));

        while(semctl(semaphore_id, 1, GETVAL, NULL) == MAX_PIZZA_AMOUNT) {};
        
        bakePizza(pizza_type);

        usleep(getRandomTime(4000, 5000));

        pizza_type = getPizzaFromOven();

        while(semctl(semaphore_id, 2, GETVAL, NULL) == MAX_PIZZA_AMOUNT) {};

        putPizzaOnTable(pizza_type);
    }
}