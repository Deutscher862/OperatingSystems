#pragma once

void my_library_function();

//row_block - table of rows
typedef struct RowBlock {
    int size; //number of rows
    char** rows_arr;
}RowBlock;

typedef struct MainArr {
    int size; //number of pairs
    int current_pair;
    RowBlock** block_arr; 
}MainArr;

RowBlock* create_row_block();

MainArr* create_main_arr(int size);

void create_blocks(MainArr* arr);

int count_file_lines(char* file );

void save_files_into_blocks(MainArr* arr);

void merge_files(MainArr* arr, char** files);

void print_files(MainArr* arr);

void remove_block(MainArr* arr, int block_ind);

void remove_row(MainArr* arr, int block_ind, int row_ind);

void free_memory(MainArr* arr);