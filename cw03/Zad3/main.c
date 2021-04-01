#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

void search_for_files(int current_depth, int max_depth, char* look_for);

int main(int argc, char **argv){
    if(argc != 4){
        printf("No nie");
        exit(-1);
    }
    
    char* first_cat = argv[1];
    char* look_for = argv[2];
    int max_depth = argv[3];
}

void search_for_files(int current_depth, int max_depth, char* look_for){
    //print all text files with given string
    printf("Entering new catalog: \n");
    system("pwd");
    printf("My PID: %d\n Found these files: ", (int)getpid());
    char mess[100];
    sprintf(mess, "ls *.txt | grep %s", look_for);
    system(mess);

    //get number of directories in current directory
    FILE* dir_counter = popen("ls -l | grep -c ^d", "r");
    if (dir_counter == NULL) {
        printf("Failed to run command\n" );
        exit(-1);
    }
    int dir_count;
    char buffer[10];
    while (fgets(buffer, sizeof(buffer), dir_counter) != NULL)
        dir_count = atoi(buffer);
    
    pclose(dir_counter);

    if(current_depth == max_depth || dir_counter == 0)
        return;
    
    //loop recursivly for directories in current directory
    for(int i = 0; i < dir_count; i++){
        pid_t new_child = fork();
        char next_directory;

        // next_directory -------------------------------------------------
        if(new_child == 0) 
            search_for_files(current_depth + 1, max_depth, look_for);
    }
    for(int i = 0; i < dir_count; i++)
        wait(NULL);
}