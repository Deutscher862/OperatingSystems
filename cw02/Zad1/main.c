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

void useSys(char* filename_1, char* filename_2, int size1, int size2);

void useLib(char* filename_1, char* filename_2, int size1, int size2);

void mergeFiles(char* buffer1, char* buffer2, int size1, int size2);

int main(int argc, char **argv){
    char* filename_1 = NULL;
    char* filename_2 = NULL;

    if(argc == 3){
        filename_1 = argv[1];
        filename_2 = argv[2];
    }
    else{
        printf("Enter first file name: ");
        fgets (filename_1, 100, stdin);
        printf("Enter second file name: ");
        fgets (filename_2, 100, stdin);
    }

    int size1 = getFileSize(filename_1);
    int size2 = getFileSize(filename_2);
    
    measure_time("Lib_File_merging", {
        useLib(filename_1, filename_2, size1, size2);
    });
    
    measure_time("Sys_File_merging", {
        useSys(filename_1, filename_2, size1, size2);
    });
    
}

void useLib(char* filename_1, char* filename_2, int size1, int size2){
    FILE* file1 = fopen(filename_1, "r");
    FILE* file2 = fopen(filename_2, "r");

    char* buffer1 = malloc(sizeof(char)*size1);
    char* buffer2 = malloc(sizeof(char)*size2);

    fread(buffer1, sizeof(char), size1, file1);
    fread(buffer2, sizeof(char), size2, file2);
    
    mergeFiles(buffer1, buffer2, size1, size2);

    free(buffer1);
    free(buffer2);

    fclose(file1);
    fclose(file2);
}

void useSys(char* filename_1, char* filename_2, int size1, int size2){
    int f1 = open(filename_1, O_RDONLY);
    int f2 = open(filename_2, O_RDONLY);
    char* buffer1 = malloc(sizeof(char)*size1);
    char* buffer2 = malloc(sizeof(char)*size2);
    int ind = 0;
    char c;
    
    while(read(f1,&c,1)==1){
        buffer1[ind] = c;
        ind++;
    }
    ind = 0;
    while(read(f2,&c,1)==1){
        buffer2[ind] = c;
        ind++;
    }
    mergeFiles(buffer1, buffer2, size1, size2);

    free(buffer1);
    free(buffer2);
    close(f1);
    close(f2);
}

void mergeFiles(char* buffer1, char* buffer2, int size1, int size2){
    int ind1 = 0;
    int ind2 = 0;
    int line_counter1 = 0;
    int line_counter2 = 0;
    while(ind1 < size1 || ind2 < size2){
        if(line_counter1 == line_counter2 || ind2 == size2 - 1){
            while(ind1 < size1 && buffer1[ind1] != '\n'){
                printf("%c", buffer1[ind1]);
                ind1++;
            }
            printf("\n");
            ind1++;
            line_counter1++;
        }
        else{
            while(ind2 < size2 && buffer2[ind2] != '\n'){
                printf("%c", buffer2[ind2]);
                ind2++;
            }
            printf("\n");
            ind2++;
            line_counter2++;
        }
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
        sprintf(str, "echo %s >> pomiar_zad_1.txt", message);
        system(str);
}