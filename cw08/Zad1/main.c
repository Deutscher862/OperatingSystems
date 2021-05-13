#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>

int width, height;
int** image;
int** new_image;

int thread_amount;
char* method;

int time_diff(clock_t t1, clock_t t2){
    return (1e6 * (t2 - t1) / sysconf(_SC_CLK_TCK));
}

int min(int a, int b) {
    return a > b ? b : a;
}

void readImage(char* input){
    FILE* file = fopen(input, "r");
    if(file == NULL)
        exit(0); 
    
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int line_counter = 0;
    int pixel_counter = 0;
    while((read = getline(&line, &len, file)) != -1){

        if(line_counter == 1){
            char* tok = strtok(line, " \n");
            width = atoi(tok);
            tok = strtok(NULL, " \n");
            height = atoi(tok);

            image = malloc(height*sizeof(int*));
            for(int i = 0; i < height; i++)
                image[i] = malloc(width*sizeof(int));

        } else if(line_counter > 3){
            char* tok = strtok(line, " \n");

            while(tok != NULL) {
                image[pixel_counter / width][pixel_counter % width] = atoi(tok);
                tok = strtok(NULL, " \n");
                pixel_counter++;
            }
        }
        line_counter++;
    }
    fclose(file);
}

void* convertImage(void* arg){
    int index = *((int*) arg);

    clock_t start = clock();

    if(strcmp(method, "sign") == 0){
        int start = index * ceil((double) 256 / thread_amount);
        int end = min((index + 1) * ceil((double) 256 / thread_amount) - 1, 256 - 1);

        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                if(image[i][j] >= start && image[i][j] <= end){
                    new_image[index][image[i][j]]++;
                }
            }
        }
    } else if(strcmp(method, "block") == 0){
        int x_min = index * ceil((double) width / thread_amount);
        int x_max = min((index + 1) * ceil((double) width / thread_amount) - 1, width - 1);

        for(int i = 0; i < height; i++) {
            for(int j = x_min; j <= x_max; j++) {
                new_image[index][image[i][j]]++;
            }
        }
    } else{
        printf("Invalid method");
        exit(1);
    }

    clock_t end = clock();

    int* time = (int*) malloc(sizeof(int));
    *time = time_diff(start, end);
    pthread_exit((void *) time);
}

void save_result_to_file(char* output) {
    FILE* file = fopen(output, "w");

    for(int i = 0; i < 256; i++) {
        int val = 0;
        for(int j = 0; j < thread_amount; j++) val += new_image[j][i];
        fprintf(file, "%d %d\n", i, val);
    }
    fclose(file);
}

int main(int argc, char** argv) {
    if(argc != 5) exit(1);

    thread_amount = atoi(argv[1]);
    method = argv[2];
    char* input = argv[3];
    char* output = argv[4];

    readImage(input);

    new_image = malloc(thread_amount*sizeof(int*));
    for(int i = 0; i < thread_amount; i++)
        new_image[i] = malloc(256*sizeof(int*));


    clock_t start = clock();

    pthread_t* threads = malloc(thread_amount*sizeof(pthread_t));
    for(int i = 0; i < thread_amount; i++) {
        int* index = (int*) malloc(sizeof(int)); /////////////// xd po co to
        *index = i;
        pthread_create(&threads[i], NULL, convertImage, (void*) index);
    }

    for(int i = 0; i < thread_amount; i++) {
        void* return_val;
        pthread_join(threads[i], &return_val); ////////////////////
        int value =  *((int*) return_val);

        printf("Thread %d time [us]: %d\n", i + 1, value);
    }

    for(int i = 0; i < height; i++)
        free(image[i]);
    free(image);

    for(int i = 0; i < height; i++)
        free(new_image[i]);
    free(new_image);
    return 0;
}