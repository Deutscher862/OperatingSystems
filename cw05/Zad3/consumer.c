#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

void saveToFile(int row, char* input, char* filename){
    FILE* file = fopen(filename, "ab");
    if(file == NULL)
        exit(1);
    printf("ZAPISUJE SE: %d %s %s\n", row, input, filename);
    fseek(file, row, SEEK_SET);
    fprintf(file,"%s", input);
    fclose(file);
}

int main(int argc, char** argv){

    if (argc != 4)
        exit(1);

    char* fifoname = argv[1];
    char* filename = argv[2];
    int N = atoi(argv[3]);

    FILE* fifo = fopen(fifoname, "r");
    if(fifo == NULL)
        exit(1);

    char buffer[N];
    while(fgets(buffer, N, fifo) != NULL){
        flock(fileno(fifo), LOCK_EX);
        char* end;
        char* input = strtok_r(buffer, "#\n", &end);
        int row = atoi(input);
        input = strtok_r(NULL, "#\n", &end);
        printf("Received: row = %d input = %s\n",row, input);
        if(input != NULL)
            saveToFile(row, input, filename);
        flock(fileno(fifo), LOCK_UN);
    }
    fclose(fifo);
    return 0;
}