#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int main(int argc, char **argv){
    char* sig_type = argv[1];

    if (strcmp(sig_type, "pending") != 0){
        raise(SIGUSR1);
    }
    if (strcmp(sig_type, "mask") == 0 || strcmp(sig_type, "pending") == 0){
        sigset_t signal_set;
        sigpending(&signal_set);
        printf("Signal %d is pending\n", sigismember(&signal_set, SIGUSR1));
    }
    printf("\n");
    return 0;
}