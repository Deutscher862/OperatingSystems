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
sigset_t suspend_mask;
union sigval value = {.sival_ptr = NULL};

void sendSignal(){
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 )
        kill(senderPID, COUNT_SIGNAL);
    else{
        sigqueue(senderPID, COUNT_SIGNAL, value);
    }
    sigsuspend(&suspend_mask);
}

void sendEndSignal(){
    if (strcmp(signal_type, "KILL") == 0  || strcmp(signal_type, "SIGRT") == 0 )
        kill(senderPID, END_SIGNAL);
    else
        sigqueue(senderPID, END_SIGNAL, value);
}

void sigusrHandler(int signum, siginfo_t *info, void *uncontext){
    waitFlag = 0;
    if(signum == COUNT_SIGNAL){
        senderPID = info->si_pid;
        printf("[CATCHER] received signal\n");
        signal_counter++;    
        sendSignal();
    }
    else{
        printf("[CATCHER] received signals: %d\n", signal_counter);
        sendEndSignal();
        exit(0);
    }
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

    //set blocking mask
    sigset_t block_mask;
    sigfillset(&block_mask);
    sigdelset(&block_mask, COUNT_SIGNAL);
    sigdelset(&block_mask, END_SIGNAL);

    if (sigprocmask(SIG_BLOCK, &block_mask, NULL) < 0)
        exit(1);

    //set signals handler
    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigusrHandler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, COUNT_SIGNAL);
    sigaddset(&act.sa_mask, END_SIGNAL);

    sigaction(COUNT_SIGNAL, &act, NULL);
    sigaction(END_SIGNAL, &act, NULL);

    //set suspend mask
    sigfillset(&suspend_mask);
    sigdelset(&suspend_mask,COUNT_SIGNAL);
    sigdelset(&suspend_mask,END_SIGNAL);

    sigsuspend(&suspend_mask);

    return 0;
}