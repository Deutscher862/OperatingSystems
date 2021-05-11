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

#include "config.h"

int semaphore_id;

int oven_memory;
int table_memory;

pid_t workers[N+M];

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

void handleSIGINT(int signum){
    for (int i = 0; i < N+M; i++)
        kill(workers[i], SIGINT);

    semctl(semaphore_id, 0, IPC_RMID, NULL);
    shmctl(oven_memory, IPC_RMID, NULL);
    shmctl(table_memory, IPC_RMID, NULL);
    
    exit(0);
}

int createSemaphore(int id){
    key_t sem_key = ftok(getenv("HOME"), id);
    int semaphore_id = semget(sem_key, 6, IPC_CREAT | 0666);
    if (semaphore_id < 0)
        errorMessage("Cannot create semaphores set");

    union semun arg;
    arg.val = 0;

    /*
     0 - czy aktualnie ktos obsługuje piec (0 - wolne, 1 - zajete)
     1 - ilosc pizz w piecu
     2 - czy aktualnie ktos obsługuje stół (0 - wolne, 1 - zajete)
     3 - ilosc pizz na stole
    */

    for (int i = 0; i < 4; i++)
        semctl(semaphore_id, i, SETVAL, arg);

    return semaphore_id;
}

int createSharedMemory(int id)
{
    key_t shm_key = ftok(getenv("HOME"), id);
    int shared_memory_id = shmget(shm_key, sizeof(pizza_memory), IPC_CREAT | 0666);
    if (shared_memory_id < 0)
        errorMessage("Cannot create shared memory");
    return shared_memory_id;
}

void fillMemory(){
    pizza_memory* oven = shmat(oven_memory, NULL, 0);
    pizza_memory* table = shmat(table_memory, NULL, 0);

    for(int i = 0; i < MAX_PIZZA_AMOUNT; i++){
        oven->values[i] = -1;
        table->values[i] = -1;
    }

    shmdt(oven);
    shmdt(table);
}

int main(){
    signal(SIGINT, handleSIGINT);

    //set semaphore
    semaphore_id = createSemaphore(0);

    //create shared_memory
    oven_memory = createSharedMemory(1);
    table_memory = createSharedMemory(2);

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

    semctl(semaphore_id, 0, IPC_RMID, NULL);
    shmctl(oven_memory, IPC_RMID, NULL);
    shmctl(table_memory, IPC_RMID, NULL);
    
    return 0;
}