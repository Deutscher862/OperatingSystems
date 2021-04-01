#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv){
    if(argc != 2)
    {
        printf("Wrong number of arguments");
        exit(-1);
    }

    int n = atoi(argv[1]);

    for(int i = 0; i < n; i++){
        pid_t child_pid = fork();
        if(child_pid != 0){
            printf("My PID number is: %d\n", (int)child_pid);
            exit(0);
        }
    }
    for(int i = 0; i < n; i++)
        wait(NULL);

    return 0;
}