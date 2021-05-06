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

int oven_descriptor;
int table_descriptor;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

int bakePizza(int pizzaType){
    /*
     * 0 - czy aktualnie ktos obsługuje piec (0 - wolne, 1 - zajete)
     * 1 - ilosc pizz w piecu
     * 2 - czy aktualnie ktos obsługuje stół (0 - wolne, 1 - zajete)
     * 3 - ilosc pizz na stole
    */
    while(semValue(0) == 1){};

    sem_post(semaphors[0]);
    sem_post(semaphors[1]);

    pizza_memory* pizzas_in_oven = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, oven_descriptor, 0);

    int pizza_id = 0;
    while(pizzas_in_oven->values[pizza_id] != -1)
        pizza_id++;

    pizzas_in_oven->values[pizza_id] = pizzaType;
    munmap(pizzas_in_oven, sizeof(pizza_memory));

    printf("(%d %ld) Dodałem pizze: %d. Liczba pizz w piecu: %d.\n", getpid(), time(NULL), pizzaType, semValue(1));

    sem_wait(semaphors[0]);

    return pizza_id;
}

int getPizzaFromOven(int pizza_id){
    while(semValue(0) == 1){};

    sem_post(semaphors[0]);

    pizza_memory* pizzas_in_oven = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, oven_descriptor, 0);
    int pizza = pizzas_in_oven->values[pizza_id];
    pizzas_in_oven->values[pizza_id] = -1;
    munmap(pizzas_in_oven, sizeof(pizza_memory));

    sem_wait(semaphors[0]);
    sem_wait(semaphors[1]);

    return pizza;
}

void putPizzaOnTable(int pizzaType){
    while(semValue(2) == 1){};

    sem_post(semaphors[2]);
    sem_post(semaphors[3]);

    pizza_memory* pizzas_on_table = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, table_descriptor, 0);

    int pizza_id = 0;
    while(pizzas_on_table->values[pizza_id] != -1)
        pizza_id++;

    pizzas_on_table->values[pizza_id] = pizzaType;
    munmap(pizzas_on_table, sizeof(pizza_memory));

    printf("(%d %ld) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d\n", getpid(), time(NULL), pizzaType, semValue(1), semValue(3));
    
    sem_wait(semaphors[2]);
}

int main(){
    srand(time(NULL));

    for(int i = 0; i < 4; i++)
        semaphors[i] = sem_open(semaphors_names[i], O_RDWR);

    oven_descriptor = shm_open(oven_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    table_descriptor = shm_open(table_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    while (1)
    {   
        int pizza_type = (rand() + getpid()) % 10;
        printf("(%d %ld) Przygotowuje pizze: %d.\n", getpid(), time(NULL), pizza_type);
        usleep(getRandomTime(1000, 2000));

        while(semValue(0) == 1 || semValue(1) == MAX_PIZZA_AMOUNT){}
            
            int pizza_id = bakePizza(pizza_type);

            usleep(getRandomTime(4000, 5000));

            pizza_type = getPizzaFromOven(pizza_id);

            while(semValue(3) == MAX_PIZZA_AMOUNT) {};

            putPizzaOnTable(pizza_type);
    }
}