#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/times.h>
#include <time.h>

int santa_sleeping = 0;

pthread_t reindeers_queue[9];
int reindeers_back = 0;
int gifts_delivered = 0;

pthread_t elves_queue[3];
int elves_with_problems = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int getRandomTime(int min, int max){
    return (rand() % (max - min + 1) + min) * 1000;
}

void* santa(void* arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        if(elves_with_problems < 3 && reindeers_back < 9) {
            printf("Mikolaj: zasypiam\n");
            santa_sleeping = 1;
            pthread_cond_wait(&cond, &mutex);
            printf("Mikolaj: budze sie\n");
            santa_sleeping = 0;
        } else if(reindeers_back == 9){
            printf("Mikołaj: dostarczam zabawki\n");
            usleep(getRandomTime(2000, 4000));
            reindeers_back = 0;
            for(int i = 0; i < 9; i++)
                reindeers_queue[i] = -1;
            gifts_delivered++;
        }
        else{
            printf("Mikolaj: rozwiazuje problemy elfow %ld %ld %ld\n", elves_queue[0], elves_queue[1], elves_queue[2]);
            usleep(getRandomTime(1000, 2000));
            elves_with_problems -= 3;
            for(int i = 0; i < 3; i++)
                elves_queue[i] = -1;
        }
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);

        if(gifts_delivered == 3) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

    }
        pthread_exit((void *) 0);
}

void* elf(void* arg) {
    pthread_t id = pthread_self();
    while(1) {
        
        if(gifts_delivered == 3)
            break;
        
        if(elves_with_problems == 3){
            for(int i = 0; i < 3; i++)
                if(elves_queue[i] == id)
                    printf("Elf: Mikołaj rozwiązuje problem, %ld\n", id);
        }
        
        usleep(getRandomTime(2000, 5000));
        pthread_mutex_lock(&mutex);
        if(elves_with_problems == 2 && santa_sleeping == 1){
            elves_queue[elves_with_problems] = id;
            elves_with_problems++;
            pthread_cond_broadcast(&cond);
            printf("Elf: wybudzam Mikolaja, %ld\n", id);
            pthread_mutex_unlock(&mutex);
        } else if(elves_with_problems < 2){
            elves_queue[elves_with_problems] = id;
            elves_with_problems++;
            printf("Elf: czeka %d elfow na Mikolaja, %ld\n", elves_with_problems, id);
            pthread_mutex_unlock(&mutex);
        } else if(gifts_delivered < 3) {
            printf("Elf: czeka na powrot elfow: %ld\n", id);
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit((void *) 0);
}

void* reindeer(void* arg) {
    pthread_t id = pthread_self();
    int queue_id = -1;
    while(1) {
        if(queue_id != -1 && reindeers_queue[queue_id] == id)
            continue;

        if(gifts_delivered == 3)
            break;

        usleep(getRandomTime(5000, 10000));
        pthread_mutex_lock(&mutex);
        queue_id = reindeers_back;
        if(reindeers_back == 9 && santa_sleeping == 1){
            reindeers_queue[reindeers_back] = id;
            reindeers_back++;
            pthread_cond_broadcast(&cond);
            printf("Renifer: wybudzam Mikolaja, %ld\n", id);
            pthread_mutex_unlock(&mutex);
        } else {
            reindeers_queue[reindeers_back] = id;
            reindeers_back++;
            printf("Renifer: czeka %d reniferow na Mikolaja, %ld\n", reindeers_back, id);
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit((void *) 0);
}

int main(int argc, char** argv) {
    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);

    pthread_t* thread_ids = (pthread_t*) calloc(20, sizeof(pthread_t));

    pthread_create(&thread_ids[0], NULL, santa, NULL);
    for(int i = 1; i < 11; i++) {
        usleep(getRandomTime(100, 1000));
        pthread_create(&thread_ids[i], NULL, elf, NULL);
    }
    
    for(int i = 11; i < 20; i++) {
        usleep(getRandomTime(100, 1000));
        pthread_create(&thread_ids[i], NULL, reindeer, NULL);
    }
    
    for(int i = 0; i < 20; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    return 0;
}