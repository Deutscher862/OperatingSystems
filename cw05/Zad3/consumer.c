#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

void saveToFile(int row, char* input, FILE* file){
    printf("Received: %d %s", row, input);
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    rewind(file);
    int len = (int)strlen(input);
    char* buffer = malloc(sizeof(char)*(size+len));
    fread(buffer, sizeof(char), size, file);
    int row_counter = 0;
    int j = 0;
    for(int i = 0; i < size+len; i++){
        char c = buffer[i+j];
        if(row_counter == row && (c == '\n' || c == '\0')){
            while(j < len){
                buffer[i+j] = input[j];
                j++;
            }
            buffer[i+j] = '\n';

        }
        if(c == '\n') row_counter++;
    }
    rewind(file);
    for(int i = 0; i < size+len; i++){
        char c = buffer[i];
        fwrite(&c, 1, sizeof(char), file);
    }   
    free(buffer);
    fclose(file);
}

int main(int argc, char** argv){

    if (argc != 4)
        exit(1);

    char *fifoname = argv[1];
    char *filename = argv[2];
    int N = atoi(argv[3]);
    FILE *file = fopen(filename, "w+");
    if (file == NULL)
        exit(1);

    FILE *fifo = fopen(fifoname, "r");
    if (fifo == NULL)
        exit(1);

    char buffer[N];
    while (fgets(buffer, N, fifo) != NULL)
    {
        char* input = strtok(buffer, "#");
        int row = atoi(input);
        input = strtok(NULL, "#");
        saveToFile(row-1, input, file);
    }
    fclose(fifo);
    fclose(file);
    return 0;
}