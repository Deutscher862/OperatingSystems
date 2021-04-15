#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int countFileLines(char* filename ){
    FILE* file = fopen(filename, "r");
    int line_counter = 0;
    char current_char;

    if (file == NULL){
        exit(-1);
    }

    for (current_char = getc(file); current_char != EOF; current_char = getc(file)){
        if (current_char == '\n')
            line_counter++;
    }
    fclose(file);
    return line_counter;
}

int main(int argc, char *argv[]){
    if(argc != 2)
        exit(1);

    char* filename = argv[1];

    FILE * file;
    char * line = NULL;
    size_t len = 0;
    int line_ind = 0;
    ssize_t read;
        
    file = fopen(filename, "r");
    if (file == NULL)
        exit(-1);

    while ((read = getline(&line, &len, file)) != -1) {
        char * lineCopy = calloc(len, sizeof(char));
        strcpy(lineCopy, line);
        line
        line_ind++;
    }
        
    fclose(file);

    return 0;
}