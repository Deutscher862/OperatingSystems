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
char* signal_type;

void sigusr1Handler(int signum){signal_counter++;}

void sigusr2Handler(int signum){
    printf("Sender received signals: %d\n", signal_counter);
    waitFlag = 0;
    }

int main(int argc, char *argv[]){
    if(argc != 4)
        exit(1);
    
    int catcherPID = atoi(argv[1]);
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

    //set mask
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, COUNT_SIGNAL);
    sigdelset(&mask, END_SIGNAL);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
        exit(1);

    //set signal handlers
    signal(COUNT_SIGNAL, sigusr1Handler);
    signal(END_SIGNAL, sigusr2Handler);

    printf("New sender PID: %d\n", getpid());

    //send signals
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 ){
        for (int i = 0; i < signal_amount; ++i)
            kill(catcherPID, COUNT_SIGNAL);
        kill(catcherPID, END_SIGNAL);
    }
    else{
        union sigval value;
        value.sival_int = 0;
        for (int i = 0; i < signal_amount; ++i)
            sigqueue(catcherPID, COUNT_SIGNAL, value);
        sigqueue(catcherPID, END_SIGNAL, value);
    }

    //receive signals
    while(waitFlag == 1) {}

    printf("\n");
    return 0;
}