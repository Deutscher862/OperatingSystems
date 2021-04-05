#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void searchForFiles(int current_depth, int max_depth, char* look_for_file, char* new_directory);

char* getOutputFromCommand(char* command);

int main(int argc, char **argv){
    if(argc != 4)
        exit(-1);   
    char* first_directory = argv[1];
    char* look_for_file = argv[2];
    int max_depth = atoi(argv[3]);
    searchForFiles(0, max_depth, look_for_file, first_directory);
    return 0;
}

void searchForFiles(int current_depth, int max_depth, char* look_for_file, char* new_directory){
    printf("\nEntering new catalog: \n");
    printf("%s\n", new_directory);
    printf("My PID: %d\nFound files:\n", (int)getpid());
    
    //print all text files with given string
    char mess[100];
    sprintf(mess, "find %s -maxdepth 1 -type f -name '*.txt' | grep %s", new_directory, look_for_file);
    system(mess);

    //get number of directories in current directory
    char mess2[100];
    sprintf(mess2, "ls %s -l | grep -c ^d", new_directory);
    int dir_counter = atoi(getOutputFromCommand(mess2));

    if(current_depth == max_depth || dir_counter == 0)
        exit(0);

    //loop recursivly for directories in current directory
    for(int i = 0; i < dir_counter; i++){
        pid_t new_child = fork();
        if(new_child == 0){
            char mess3[100];
            sprintf(mess3, "ls %s -l | grep ^d | head -%d | tail -1 | awk '{print $9}'",new_directory, i+1);
            char* next_directory = new_directory;
            sprintf(next_directory, "%s/%s", new_directory, getOutputFromCommand(mess3));
            searchForFiles(current_depth + 1, max_depth, look_for_file, next_directory);
            exit(0);
        }
    }
    for(int i = 0; i < dir_counter; i++)
        wait(NULL);
}

char* getOutputFromCommand(char* command){
    //return char* with terminal command output
    FILE* output_file = popen(command, "r");
    if (output_file == NULL) {
        printf("Failed to run command\n");
        exit(-1);
    }
    else{
        char buffer[100];
        fgets(buffer, sizeof(buffer), output_file);
        buffer[strcspn(buffer, "\n")] = 0;
        char *result = buffer;
        pclose(output_file);
        return result;
    }
}
