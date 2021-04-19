#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>

char* P_FILES[] = {"p1.txt", "p2.txt", "p3.txt", "p4.txt", "p5.txt",};

void test1(){
    printf("[5 PRODUCENTS - 1 CONSUMER]\n");

    char *consumer[] = {"./consumer", "fifo", "c.txt", "3", NULL};
    if (fork() == 0)
        execvp(consumer[0], consumer);

    for(int i = 0; i < 5; i++){
        if(fork()==0){
            char row_number[2];
            sprintf(row_number, "%d", i);
            char *producer[] = {"./producer", "fifo", row_number, P_FILES[i], "3", NULL};
            execvp(producer[0], producer);
        }
    }

    for (int i = 0; i < 6; i++)
        wait(NULL);
}

void test2(){
    printf("[1 PRODUCENT - 5 CONSUMERS]\n");

     for(int i = 0; i < 5; i++){
        if(fork()==0){
            char *consumer[] = {"./consumer", "fifo", "c.txt", "3", NULL};
            execvp(consumer[0], consumer);
        }
    }
    
    char *producer[] = {"./producer", "fifo", "1", "p1.txt", "5", NULL};
    if (fork() == 0)
        execvp(producer[0], producer);

    for (int i = 0; i < 10; i++)
        wait(NULL);
}

void test3(){
    printf("[5 PRODUCENTS - 5 CONSUMERS]\n");

     for(int i = 0; i < 5; i++){
        if(fork()==0){
            char *consumer[] = {"./consumer", "fifo", "c.txt", "5", NULL};
            execvp(consumer[0], consumer);
        }
    }

    for(int i = 0; i < 5; i++){
        if(fork()==0){
            char row_number[2];
            sprintf(row_number, "%d", i);
            char *producer[] = {"./producer", "fifo", row_number, P_FILES[i], "3", NULL};
            execvp(producer[0], producer);
        }
    }

    for (int i = 0; i < 6; i++)
        wait(NULL);
}

int main(int argc, char *argv[])
{
    mkfifo("fifo", S_IRUSR | S_IWUSR);

    //test1();
    //test2();
    test3();

    return 0;
}