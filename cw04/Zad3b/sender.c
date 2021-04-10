#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int signal_amount;
int signal_counter = 0;
int waitFlag = 1;
int COUNT_SIGNAL;
int END_SIGNAL;
int catcherPID;
char* signal_type;
sigset_t suspend_mask;
union sigval value = {.sival_ptr = NULL};

void sendSignal(){
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 ){
        kill(catcherPID, COUNT_SIGNAL);
    }        
    else{
        sigqueue(catcherPID, COUNT_SIGNAL, value);
    }
    sigsuspend(&suspend_mask);
}

void sendEndSignal(){
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 )
        kill(catcherPID, END_SIGNAL);
    else
        sigqueue(catcherPID, END_SIGNAL, value);
    sigsuspend(&suspend_mask);
}

void sigusr1Handler(int signum){
    printf("[SENDER] received signal\n");
    waitFlag = 0;
    signal_counter++;
    if(signal_counter == signal_amount)
        sendEndSignal();
    else{
        sendSignal();
    }
}

void sigusr2Handler(int signum){
    printf("[SENDER] received signals: %d\n", signal_counter);
    exit(0);
    }

int main(int argc, char *argv[]){
    if(argc != 4)
        exit(1);
    
    catcherPID = atoi(argv[1]);
    signal_amount = atoi(argv[2]);
    char* sig_type = argv[3];

    //set signal type
    if (strcmp(sig_type, "KILL") == 0){
        signal_type = "KILL";
        COUNT_SIGNAL = SIGUSR1;
        END_SIGNAL = SIGUSR2;
    }
    else if (strcmp(sig_type, "SIGQUEUE") == 0){
        signal_type = "SIGQUEUE";
        COUNT_SIGNAL = SIGUSR1;
        END_SIGNAL = SIGUSR2;
    }
    else if (strcmp(sig_type, "SIGRT") == 0){
        signal_type = "SIGRT";
        COUNT_SIGNAL = SIGRTMIN + 1;
        END_SIGNAL = SIGRTMIN + 2;
    }
    else exit(1);

    //set blocking mask
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, COUNT_SIGNAL);
    sigdelset(&mask, END_SIGNAL);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
        exit(1);

    //set signal handlers
    signal(COUNT_SIGNAL, sigusr1Handler);
    signal(END_SIGNAL, sigusr2Handler);

    //set suspend mask
    sigfillset(&suspend_mask);
    sigdelset(&suspend_mask,COUNT_SIGNAL);
    sigdelset(&suspend_mask,END_SIGNAL);

    sendSignal();

    printf("\n");
    return 0;
}