#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/times.h"
#include "unistd.h"
#include "fcntl.h"	
#include "time.h"

static struct tms tms_start, tms_end;
static clock_t clock_start, clock_end;
char message[100];

#define measure_time(operation, code_block)\
    clock_start = times(&tms_start);\
    code_block\
    clock_end = times(&tms_end);\
    sprintf(message, "Measuring time of %s results:", operation);\
    saveToReport(message);\
    sprintf(message, "real time: %fs", (double)(clock_end - clock_start)/ CLOCKS_PER_SEC);\
    saveToReport(message);\
    sprintf(message, " sys time: %fs", (double)(tms_end.tms_stime - tms_start.tms_stime)/ CLOCKS_PER_SEC);\
    saveToReport(message);\
    sprintf(message, "user time: %fs", (double)(tms_end.tms_utime - tms_start.tms_utime)/ CLOCKS_PER_SEC);\
    saveToReport(message);\
    saveToReport("");

void saveToReport(char* message);

int getFileSize(char* filename);

void useLib(char chosen_char, char * filename, int size);

void useSys(char chosen_char, char * filename, int size);

void printFile(char chosen_char, char* buffer, int size);

int main(int argc, char **argv){

    if(argc != 3) exit(-1);
    char chosen_char = argv[1][0];
    char* filename = argv[2];
    int size = getFileSize(filename);
    
    measure_time("Lib_print_lines_with_chosen_char", {
        useLib(chosen_char, filename, size);
    });

    measure_time("Sys_print_lines_with_chosen_char", {
        useSys(chosen_char, filename, size);
    });
}

void useLib(char chosen_char, char * filename, int size){
    FILE* file = fopen(filename, "r");
    char* buffer = malloc(sizeof(char)*size);

    fread(buffer, sizeof(char), size, file);
    printFile(chosen_char, buffer, size);

    free(buffer);
    fclose(file);
}

void useSys(char chosen_char, char * filename, int size){
    int file = open(filename, O_RDONLY);
    char* buffer = malloc(sizeof(char)*size);
    int ind = 0;
    char c;
    
    while(read(file,&c,1)==1){
        buffer[ind] = c;
        ind++;
    }
    printFile(chosen_char, buffer, size);
    free(buffer);
    close(file);
}

void printFile(char chosen_char, char* buffer, int size){
    for(int i = 0; i < size; i++){          
        int line_length = 0;
        //containts_char - like bool: 0-false, 1-true
        int contains_char = 0;

        //go through whole line and check if chosen char occurs
        while(i + line_length < size && buffer[i+line_length] != '\n'){
            if(buffer[i+line_length] == chosen_char)
                contains_char = 1;
            line_length++;
        }
            
        if(contains_char == 1){
            int j = i;
            while(j <= i + line_length){
                printf("%c", buffer[j]);
                j++;
            }
        }
        i = i + line_length;
    }
}

int getFileSize(char* filename){
    FILE* file = fopen(filename, "r");
    if(file == NULL)
        exit(-1);
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fclose(file);
    return size;
}

void saveToReport(char* message){
        char str[100];
        sprintf(str, "echo %s >> pomiar_zad_2.txt", message);
        system(str);
}