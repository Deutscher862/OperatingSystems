#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

void errorMessage(char* message){
    printf("Error: %s\n", message);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

int getSemaphore(int id)
{
    key_t sem_key = ftok(getenv("HOME"), id);
    int semaphore_id = semget(sem_key, 0, 0);
    if (semaphore_id < 0)
        errorMessage("Can't get semaphore"); 
    return semaphore_id;
}

int getSharedMemory(int id)
{
    key_t shm_key = ftok(getenv("HOME"), id);
    int shared_memory_id = shmget(shm_key, 0, 0);
    if (shared_memory_id < 0)
        errorMessage("Can't access shared memory");
    return shared_memory_id;
}