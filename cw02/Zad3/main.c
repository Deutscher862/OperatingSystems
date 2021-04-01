#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
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

void useLib(int size);

void useSys(int size);

void saveToReport(char* message);

int checkIfPerfectSquare(int number);

int getFileSize(char* filename);

void saveToFile(char* chars_to_save, int length, FILE* lib_output, int sys_output);

void checkNumbers(int size, char* buffer, FILE* a_lib_output, FILE* b_lib_output, FILE* c_lib_output, int a_sys_output, int b_sys_output, int c_sys_output);

int main(){

    int size = getFileSize("dane.txt");
    
    measure_time("Lib_numbers_checking", {
        useLib(size);
    });

    measure_time("Sys_numbers_checking", {
        useSys(size);
    });
}

void useLib(int size){
    FILE* input = fopen("dane.txt", "r");
    FILE* a_output= fopen("a.txt", "w");
    FILE* b_output= fopen("b.txt", "w");
    FILE* c_output= fopen("c.txt", "w");
    char* buffer = malloc(sizeof(char)*size);

    fread(buffer, sizeof(char), size, input);
    checkNumbers(size, buffer, a_output, b_output, c_output, -1, -1, -1);

    free(buffer);
    fclose(input);
    fclose(a_output);
    fclose(b_output);
    fclose(c_output);
}

void useSys(int size){
    int input = open("dane.txt", O_RDONLY);
    int a_output = open("a.txt", O_WRONLY);
    int b_output = open("b.txt", O_WRONLY);
    int c_output = open("c.txt", O_WRONLY);
    char* buffer = malloc(sizeof(char)*size);
    int ind = 0;
    char c;

    while(read(input,&c,1)==1){
        buffer[ind] = c;
        ind++;
    }
    checkNumbers(size, buffer, NULL, NULL, NULL, a_output, b_output, c_output);
    free(buffer);
    close(input);
    close(a_output);
    close(b_output);
    close(c_output);
}

void checkNumbers(int size, char* buffer, FILE* a_lib_output, FILE* b_lib_output, FILE* c_lib_output, int a_sys_output, int b_sys_output, int c_sys_output){
    for(int i = 0; i < size; i++){
            int digits_counter = 0;
            while(i < size && buffer[i+digits_counter] != '\n'){
                digits_counter++;
            }
            
            char* chars_to_save = malloc(sizeof(char)*(digits_counter));
            int number = atoi(&buffer[i]);
            sprintf(chars_to_save, "%d\n", number);

            if(number % 2 == 0){
                saveToFile(chars_to_save, digits_counter, a_lib_output, a_sys_output);
            }
            
            int r = (number/10) % 10;
            if(digits_counter > 1 && (r == 0 || r == 7)){
                saveToFile(chars_to_save, digits_counter, b_lib_output, b_sys_output);
            }

            if(checkIfPerfectSquare(number) == 1){
                saveToFile(chars_to_save, digits_counter, c_lib_output, c_sys_output);
            }
                
            i = i + digits_counter;
            free(chars_to_save);
        }
}

int checkIfPerfectSquare(int number){
    long double sr = sqrt(number);
    if(sr * sr == number)
        return 1;
    else return 0;
}

void saveToFile(char* chars_to_save, int length, FILE* lib_output, int sys_output){
    char c;
    for(int i = 0; i <= length; i++){
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
        sprintf(str, "echo %s >> pomiar_zad_3.txt", message);
        system(str);
}