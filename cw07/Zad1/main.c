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

int shared_memory_id;

pid_t workers[N+M];

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

void clear(){
    semctl(semaphore_id, 0, IPC_RMID, NULL);
    shmctl(shared_memory_id, IPC_RMID, NULL);
    system("make clean");
}

void handleSIGINT(int signum){
    for (int i = 0; i < N+M; i++)
        kill(workers[i], SIGINT);

    clear();
    exit(0);
}

int createSemaphore(int id){
    key_t sem_key = ftok(getenv("HOME"), id);
    int semaphore_id = semget(sem_key, 6, IPC_CREAT | 0666);
    if (semaphore_id < 0)
        errorMessage("Cannot create semaphores set");

    union semun arg;
    arg.val = 0;

    for (int i = 0; i < 6; i++)
        semctl(semaphore_id, i, SETVAL, arg);

    return semaphore_id;
}

int createSharedMemory(int id)
{
    key_t shm_key = ftok(getenv("HOME"), id);
    int shared_memory_id = shmget(shm_key, sizeof(pizzas), IPC_CREAT | 0666);
    if (shared_memory_id < 0)
        errorMessage("Cannot create shared memory");
    return shared_memory_id;
}

int main(){
    //set semaphore
    semaphore_id = createSemaphore(0);

    //create shared_memory
    shared_memory_id = createSharedMemory(1);

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

    clear();
    return 0;
}