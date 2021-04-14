#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

void handler(int signum){
    printf("Signal received\n");
}

void checkPending(char* sig_type, sigset_t signal_set){
    if (strcmp(sig_type, "mask") == 0 || strcmp(sig_type, "pending") == 0){
        sigpending(&signal_set);
        printf("Signal %d is pending\n", sigismember(&signal_set, SIGUSR1));
    }
}

int main(int argc, char **argv){

    if(argc == 1 || argc > 3)
        exit(1);

    char* sig_type = argv[1];

    if(strcmp(sig_type, "ignore") == 0)
        signal(SIGUSR1, SIG_IGN);

    else if(strcmp(sig_type, "handler") == 0)
        signal(SIGUSR1, handler);
    
    else if (strcmp(sig_type, "mask") == 0 || strcmp(sig_type, "pending") == 0){
        sigset_t signal_set;
        //init empty signal set
        sigemptyset(&signal_set);
        //add user singal to set
        sigaddset(&signal_set, SIGUSR1);
        //set new mask
        if (sigprocmask(SIG_BLOCK, &signal_set, NULL) < 0)
            perror("Signal blocking failed");
    }
    raise(SIGUSR1);
    sigset_t signal_set;

    checkPending(sig_type, signal_set);
    
    if (strcmp(argv[2], "exec") == 0)
        execl("./exec", "./exec", sig_type, NULL);
    else{
        if (fork() == 0){
            if (strcmp(sig_type, "pending") != 0)
                raise(SIGUSR1);
            checkPending(sig_type, signal_set);
        }
        wait(NULL);
    }
    printf("\n");
    return 0;
}