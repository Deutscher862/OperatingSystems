#include "my_library.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

RowBlock *create_row_block(){
    RowBlock *res = malloc(sizeof(RowBlock));
    return res;
}

MainArr *create_main_arr(int size){
    //size - number of pairs of files
    MainArr *res = malloc(sizeof(MainArr));
    res->size = size;
    res->current_pair = 0;
    res->block_arr = (RowBlock **) calloc(size, sizeof(RowBlock *));
    return res;
}

void create_blocks(MainArr* arr){
    for(int i = 0; i < arr->size; i++){
        arr->block_arr[i] = create_row_block();
    }
}

int count_file_lines(char* filename ){
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

void save_files_into_blocks(MainArr* arr){
    char filename[100];
    //iterating through all merged files
    for(int i = 0; i < arr->size; i++){
        snprintf(filename, sizeof(filename), "tmp%d.txt", i);

        int number_od_rows = count_file_lines(filename);
        RowBlock* block = arr->block_arr[i];

        if(block == NULL) exit(-1);

        block->size = number_od_rows;
        block->rows_arr = (char **) calloc(number_od_rows,sizeof(char *));
        
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
            block->rows_arr[line_ind] = lineCopy;
            line_ind++;
        }
        
        fclose(file);
    }
}

void merge_files(MainArr* arr, char** files){
    int current_pair = arr->current_pair;
    FILE* file1 = fopen(files[0], "r");
    FILE* file2 = fopen(files[1], "r");

    if(file1 != NULL && file2 != NULL){
        char str[100];
        sprintf(str, "paste %s %s -d '\\n' | grep -vE \"$^\" > tmp%d.txt",
        files[0], files[1], current_pair);
        system(str);
        fclose(file1);
        fclose(file2);
    }
    else exit(-1);
    arr->current_pair = arr->current_pair + 1;
}

void print_files(MainArr* arr){
    for(int i =0; i < arr->size; i++){
        printf("File %d\n", i);
        RowBlock* current_block = arr->block_arr[i];

        if(current_block != NULL){
            for(int j = 0; j < current_block->size; j++){
                char* current_row = current_block->rows_arr[j];
                if(current_row != NULL)
                    printf("%s\n", current_block->rows_arr[j]);
            } 
        }
    }
}

void free_memory(MainArr* arr){
    for(int i = 0; i < arr->size; i++){
        if(arr->block_arr[i] != NULL)
            remove_block(arr, i);
    }
    free(arr);
    arr = NULL;
}

void remove_block(MainArr* arr, int block_ind){
    RowBlock* current_block = arr->block_arr[block_ind];

    if(current_block == NULL) return;

    for(int i = 0; i < current_block->size; i++){
        free(current_block->rows_arr[i]);
        current_block->rows_arr[i] = NULL;
    }

    free(arr->block_arr[block_ind]);
    arr->block_arr[block_ind] = NULL;
}

void remove_row(MainArr* arr, int block_ind, int row_ind){
    char* current_row = arr->block_arr[block_ind]->rows_arr[row_ind];
    if(current_row == NULL) return;

    free(arr->block_arr[block_ind]->rows_arr[row_ind]);
    arr->block_arr[block_ind]->rows_arr[row_ind] = NULL;
}