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
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "config.h"

pid_t workers[N+M];

void handleSIGINT(int signum){
    for (int i = 0; i < N+M; i++)
        kill(workers[i], SIGINT);

    for(int i = 0; i < 4; i++)
        sem_unlink(semaphors_names[i]);
    
    shm_unlink(oven_name);
    shm_unlink(table_name);
    
    exit(0);
}

void createSemaphore(){
    sem_t* sem;
    for(int i = 0; i < 4; i++) {
        sem = sem_open(semaphors_names[i], O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, 0);
        sem_close(sem);
    }
}

void fillMemory(){
    int oven_descriptor = shm_open(oven_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    pizza_memory* oven = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, oven_descriptor, 0);

    int table_descriptor = shm_open(table_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    pizza_memory* table = mmap(NULL, sizeof(pizza_memory), PROT_READ | PROT_WRITE, MAP_SHARED, table_descriptor, 0);

    for(int i = 0; i < MAX_PIZZA_AMOUNT; i++){
        oven->values[i] = -1;
        table->values[i] = -1;
    }

    munmap(oven, sizeof(pizza_memory));
    munmap(table, sizeof(pizza_memory));
}

int main(){
    signal(SIGINT, handleSIGINT);

    //set semaphore
    createSemaphore();

    //create shared_memory
    int oven_dc = shm_open(oven_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    ftruncate(oven_dc, sizeof(pizza_memory));
    
    int table_dc = shm_open(table_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    ftruncate(table_dc, sizeof(pizza_memory));

    fillMemory();

    //run cooks
    for (int i = 0; i < N; i++){
        pid_t child_pid = fork();
        if (child_pid == 0)
            execlp("./cook", "cook", NULL);

        workers[i] = child_pid;
    }

    //run deliverers
    for (int i = 0; i < M; i++){
        pid_t child_pid = fork();
        if (child_pid == 0)
            execlp("./deliverer", "deliverer", NULL);
            
        workers[N+i] = child_pid;
    }

    for (int i = 0; i < N+M; i++)
        wait(NULL);


    for(int i = 0; i < 4; i++)
        sem_unlink(semaphors_names[i]);
    
    shm_unlink(oven_name);
    shm_unlink(table_name);
    
    return 0;
}