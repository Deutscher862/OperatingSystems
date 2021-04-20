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

    int fifo = open(fifoname, O_WRONLY);
    if(fifo < 1)
        exit(1);

    FILE* file = fopen(filename, "r");
    if(file == NULL)
        exit(1);

    char buffer[N];
    srand(time(NULL));
    while(fgets(buffer, N, file) != NULL){
        char line[N+strlen(row)+3];
        sprintf(line, "#%d#%s\n", atoi(row), buffer);
        write(fifo, line, strlen(line));
        sleep(rand()%2);
    }
    close(fifo);
    fclose(file);
    return 0;
}