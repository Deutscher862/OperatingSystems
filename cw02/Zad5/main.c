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

void useLib(int size, char* filename);

void useSys(int size, char* filename);

void saveFileWithChangedWords(int size, char* buffer, FILE* lib_output, int sys_output);

void writeToFile(char* chars_to_save, int length, FILE* lib_output, int sys_output);

int main(int argc, char **argv){
    if(argc != 2) exit(-1);
    char * filename = argv[1];
    int size = getFileSize(filename);
    
    measure_time("Lib_cut_long_lines", {
        useLib(size, filename);
    });
    
    measure_time("Sys_cut_long_lines", {
        useSys(size, filename);
    });
}

void useLib(int size, char* filename){
    FILE* input = fopen(filename, "r");
    FILE* output= fopen("result.txt", "w");
    char* buffer = malloc(sizeof(char)*size);

    fread(buffer, sizeof(char), size, input);
    saveFileWithChangedWords(size, buffer, output, -1);

    free(buffer);
    fclose(input);
    fclose(output);
}

void useSys(int size, char* filename){
    int input = open(filename, O_RDONLY);
    int output = open("result.txt", O_WRONLY);
    char* buffer = malloc(sizeof(char)*size);
    int ind = 0;
    char c;

    while(read(input,&c,1)==1){
        buffer[ind] = c;
        ind++;
    }
    saveFileWithChangedWords(size, buffer, NULL, output);
    free(buffer);
    close(input);
    close(output);
}

void saveFileWithChangedWords(int size, char* buffer, FILE* lib_output, int sys_output){
    char c;
    int line_chars = 0;
    for(int i = 0; i < size; i++){
        c = buffer[i];
        //iterate through file and count chars
        //if counter increases to 50, add new line char and reset counter 
        writeToFile(&c, 1, lib_output, sys_output);
        line_chars++;
        if(line_chars == 50){
            c = '\n';
            writeToFile(&c, 1, lib_output, sys_output);
            line_chars = 0;
        }
    }
}

void writeToFile(char* chars_to_save, int length, FILE* lib_output, int sys_output){
    char c;
    for(int i = 0; i < length; i++){
        c = chars_to_save[i];
        if(lib_output != NULL)
            fwrite(&c, 1, sizeof(char), lib_output);
        else write(sys_output, &c, 1);
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
        sprintf(str, "echo %s >> pomiar_zad_5.txt", message);
        system(str);
}