#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

void saveToFile(char* message, char* filename){
    char* input = strtok(message, "#");
    int row = atoi(input);
    input = strtok(NULL, "#");
    printf("Received: %d %s", row, input);

    FILE *file = fopen(filename, "ab+");
    if (file == NULL)
        exit(1);
    
    char *line = NULL;
    size_t len = 0;
    int line_counter = 0;
    int flag;
    while(getline(&line, &len, file) != -1) {
        if(line_counter == row)
            flag = ftell(file);
        line_counter++;
    }
    fseek(file, flag, SEEK_SET);
    char c;
    for(int i = 0; i < strlen(input); i++){
        c = input[i];
        fwrite(&c, 1, sizeof(char), file);
    }   
    fclose(file);
}

int main(int argc, char** argv){

    if (argc != 4)
        exit(1);

    char *fifoname = argv[1];
    char *filename = argv[2];
    int N = atoi(argv[3]);

    FILE *tmp = fopen("tmp.txt", "ab+");
    if (tmp == NULL)
        exit(1);

    FILE *fifo = fopen(fifoname, "r");
    if (fifo == NULL)
        exit(1);

    char buffer[N];
    while (fgets(buffer, N, fifo) != NULL)
    {
        fprintf(tmp, buffer, strlen(buffer));
    }
    fclose(fifo);

    char *line = NULL;
    size_t len = 0;
    rewind(tmp);
    while(getline(&line, &len, tmp) != -1) {
        saveToFile(line, filename);
    }
    fclose(tmp);
    return 0;
}