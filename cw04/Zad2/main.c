#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

void siginfoHandler(int signum, siginfo_t* info, void* context);

void resethandHandler(int signum);

void nocldstopHandler(int signum);

void siginfoUSR1Test();

void siginfoINTTest();

void siginfoCHLDTest();

void resethandTest();

void nocldstopTest();

int main(int argc, char** argv){
    if(argc != 2)
        exit(1);

    char* flag_type = argv[1];
    if (strcmp(flag_type, "SIGINFO") == 0){
        siginfoUSR1Test();
        siginfoINTTest();
        siginfoCHLDTest();
    }
    else if(strcmp(flag_type, "RESETHAND") == 0){
        resethandTest();
    }
    else if(strcmp(flag_type, "NOCLDSTOP") == 0){
        nocldstopTest();
    }
    return 0;
}

void siginfoHandler(int signum, siginfo_t* info, void* context){
    printf("Signal number: %d\n", info->si_signo);
    printf("Signal process ID: %d\n", info->si_pid);
    printf("Real user ID: %d\n", info->si_uid);
    printf("POSIX timer overrun count: %d\n", info->si_overrun	);
    printf("Exit value: %d\n", info->si_status);
}

void resethandHandler(int signum){
    printf("This message will by wrote only once\n");
}

void nocldstopHandler(int signum){
    printf("Currently handling process: %d\n", signum);
}

void siginfoUSR1Test(){
    printf("SIGUSR1 siginfo test\n");
    struct sigaction act;
    act.sa_sigaction = siginfoHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    raise(SIGUSR1);
    printf("\n");
}

void siginfoINTTest(){
    printf("SIGINT siginfo test\n");
    struct sigaction act;
    act.sa_sigaction = siginfoHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
    raise(SIGINT);
    printf("\n");
}

void siginfoCHLDTest(){
    printf("SIGCHLD siginfo test\n");
    struct sigaction act;
    act.sa_sigaction = siginfoHandler;
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);
    if (fork() == 0){
        exit(1);
    }
    wait(NULL);
}

void resethandTest(){
    printf("resethand test\n");
    struct sigaction act;
    act.sa_handler = resethandHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESETHAND;
    sigaction(SIGUSR1, &act, NULL);
    if (fork() == 0){
        raise(SIGUSR1);
        raise(SIGUSR1);
    }
    wait(NULL);
    printf("\n");
}

void nocldstopTest(){
    printf("nocldstop test\n");
    struct sigaction act;
    act.sa_handler = nocldstopHandler;
    act.sa_flags = SA_NOCLDSTOP;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, NULL);
    if (fork() == 0){
        raise(SIGSTOP);
    }
    printf("\n");
}