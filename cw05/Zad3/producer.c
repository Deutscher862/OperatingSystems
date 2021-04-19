#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>

int main(int argc, char** argv){
    if(argc != 5)
        exit(1);

    char* fifoname = argv[1];
    char* row = argv[2];
    char* filename = argv[3];
    int N = atoi(argv[4]);

    FILE* fifo = fopen(fifoname, "w");
    if(fifo == NULL)
        exit(1);

    FILE* file = fopen(filename, "r");
    if(file == NULL)
        exit(1);

    char buffer[N];
    srand(time(NULL));
    while (fgets(buffer, N, file)){
        flock(fileno(fifo), LOCK_EX);
        char line[N+3+strlen(row)];
        sprintf(line, "#%d#%s\n", atoi(row), buffer);
        printf("Sending: %s\n", line);
        fputs(line, fifo);
        flock(fileno(fifo), LOCK_UN);
        sleep((rand()%2));
    }
    fclose(fifo);
    fclose(file);
    return 0;
}