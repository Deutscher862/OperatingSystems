#include "my_library.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/times.h>
#include <unistd.h>
#include <time.h>

static struct tms tms_start, tms_end;
static clock_t clock_start, clock_end;
char message[100];

#define measure_time(operation, code_block)\
    clock_start = times(&tms_start);\
    code_block\
    clock_end = times(&tms_end);\
    sprintf(message, "Measuring time of %s results:", operation);\
    save_to_raport(message);\
    sprintf(message, "real time: %fs", (double)(clock_end - clock_start)/ CLOCKS_PER_SEC);\
    save_to_raport(message);\
    sprintf(message, " sys time: %fs", (double)(tms_end.tms_stime - tms_start.tms_stime)/ CLOCKS_PER_SEC);\
    save_to_raport(message);\
    sprintf(message, "user time: %fs", (double)(tms_end.tms_utime - tms_start.tms_utime)/ CLOCKS_PER_SEC);\
    save_to_raport(message);\
    save_to_raport("");

void save_to_raport(char* message){
        char str[100];
        sprintf(str, "echo %s >> raport2.txt", message);
        system(str);
}

int main(int argc, char **argv){
    MainArr* main_arr = NULL;

    //argv[1] = number of pairs
    int number_of_pairs = atoi(argv[1]);

    //argv[2] = test_type
    sprintf(message, "Test_type %s results:", argv[2]);\
    save_to_raport(message);
    save_to_raport("");

    for(int i = 3; i < argc; i++){
        char* arg = argv[i];

        if(strcmp(arg, "create_table") == 0) {
            if(main_arr) main_arr = NULL;
            measure_time("create_table", {
            main_arr = create_main_arr(number_of_pairs);
            create_blocks(main_arr); 
            });
        }

        else if (strcmp(arg, "merge_files") == 0) {
            if(main_arr){
                //merging
                char* files[2];
                measure_time("merge_files", {
                    for(int j = 0; j < main_arr->size; j++){
                        files[0] = strtok(argv[++i], ":");
                        files[1] = strtok(NULL, "");
                        merge_files(main_arr, files);
                    }
                });

                //saving
                measure_time("save_files_into_blocks", {
                    save_files_into_blocks(main_arr);
                }); 
            }
        }

        else if (strcmp(arg, "remove_block") == 0) {
            if(main_arr){
                int remove_count = atoi(argv[++i]);
                measure_time("remove_blocks", {
                    for(int j = 0; j < remove_count; j++){
                        int block_index = atoi(argv[++i]);
                        remove_block(main_arr, block_index);
                        main_arr->block_arr[block_index] = create_row_block();
                    }
                });
            }
        }

        else if (strcmp(arg, "remove_row") == 0) {
            if(main_arr){
                int block_index = atoi(argv[++i]);
                int row_index = atoi(argv[++i]);
                measure_time("save_files_into_blocks", {
                    remove_row(main_arr, block_index, row_index);
                }); 
            }
        }
    }

    free_memory(main_arr);

    save_to_raport("------------------------------------------");
    save_to_raport("");
};