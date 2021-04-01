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

void useLib(char* change_from, char*change_to, int size, char* filename);

void useSys(char* change_from, char*change_to, int size, char* filename);

void saveFileWithChangedWords(char* change_from, char*change_to, int size, char* buffer, FILE* lib_output, int sys_output);

void writeToFile(char* chars_to_save, int length, FILE* lib_output, int sys_output);

int main(int argc, char **argv){
    if(argc != 4) exit(-1);
    char * filename = argv[1];
    char* change_from = argv[2];
    char* change_to = argv[3];
    int size = getFileSize(filename);
    
    measure_time("Lib_save_file_with_changed_words", {
        useLib(change_from, change_to, size, filename);
    });
    
    measure_time("Sys_save_file_with_changed_words", {
        useSys(change_from, change_to, size, filename);
    });
    
}

void useLib(char* change_from, char*change_to, int size, char* filename){
    FILE* input = fopen(filename, "r");
    FILE* output= fopen("result.txt", "w");
    char* buffer = malloc(sizeof(char)*size);

    fread(buffer, sizeof(char), size, input);
    saveFileWithChangedWords(change_from, change_to, size, buffer, output, -1);

    free(buffer);
    fclose(input);
    fclose(output);
}

void useSys(char* change_from, char*change_to, int size, char* filename){
    int input = open(filename, O_RDONLY);
    int output = open("result.txt", O_WRONLY);
    char* buffer = malloc(sizeof(char)*size);
    int ind = 0;
    char c;

    while(read(input,&c,1)==1){
        buffer[ind] = c;
        ind++;
    }
    saveFileWithChangedWords(change_from, change_to, size, buffer, NULL, output);
    free(buffer);
    close(input);
    close(output);
}

void saveFileWithChangedWords(char* change_from, char*change_to, int size, char* buffer, FILE* lib_output, int sys_output){
    char c;
    int change_from_len = strlen(change_from);
    int change_to_len = strlen(change_to);
    for(int i = 0; i < size; i++){
        //check if current char equals first char of chosen word
        if(buffer[i] == change_from[0]){
            int counter = 0;
            //chceck if next chars are equal
            while(i+counter < size && counter < change_from_len && buffer[i+counter] == change_from[counter]){
                counter++;
            }
            //if chosen word occurs, write it to file
            if(counter == change_from_len){
                writeToFile(change_to, change_to_len, lib_output, sys_output);
                i = i + counter - 1;
            }
            else{
                c = buffer[i];
                writeToFile(&c, 1, lib_output, sys_output);
            }
        }
        else{
            c = buffer[i];
            writeToFile(&c, 1, lib_output, sys_output);
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
        sprintf(str, "echo %s >> pomiar_zad_4.txt", message);
        system(str);
}