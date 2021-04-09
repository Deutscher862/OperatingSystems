#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int signal_counter = 0;
int COUNT_SIGNAL;
int END_SIGNAL;
int senderPID;
int waitFlag = 1;
char* signal_type;

void sigusr1Handler(int signum){signal_counter++;}

void sigusr2Handler(int signum, siginfo_t *info, void *uncontext){
    printf("Catcher received signals: %d\n", signal_counter);
    senderPID = info->si_pid;
    waitFlag = 0;
}

int main(int argc, char *argv[]){
    if(argc != 2)
        exit(1);
    
    char* sig_type = argv[1];
    
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
    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, COUNT_SIGNAL);
    sigdelset(&block_mask, END_SIGNAL);

    if (sigprocmask(SIG_BLOCK, &block_mask, NULL) < 0)
        exit(1);

    //set signal handlers
    signal(COUNT_SIGNAL, sigusr1Handler);
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigusr2Handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, END_SIGNAL);
    sigaction(END_SIGNAL, &act, NULL);

    //receive signals
    while (waitFlag == 1) {}

    //send signals
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 ){
        for (int i = 0; i < signal_counter; ++i)
            kill(senderPID, COUNT_SIGNAL);
        kill(senderPID, END_SIGNAL);
    }
    else{
        union sigval value;
        value.sival_int = 0;
        for (int i = 0; i < signal_counter; ++i)
            sigqueue(senderPID, COUNT_SIGNAL, value);
        sigqueue(senderPID, END_SIGNAL, value);
    }

    return 0;
}