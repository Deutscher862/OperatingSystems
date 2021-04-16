#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>

#define READ 0
#define WRITE 1

int getComponentId(char* line, int i){
    if (i == 0) return atoi(&line[8]);
    else return atoi(&line[9]);
}

int* getComponentsId(char* line){
    //return arr of components for current command line
    char** components = (char**)calloc(20, sizeof(char*));
    char* arg = strtok(line, "|");

    int counter = 0;
    components[counter++] = arg;

    while ((arg = strtok(NULL, "|")) != NULL){
        components[counter++] = arg;
    }

    int* componentsId = (int*) calloc(20, sizeof(int));
    int i = 0;
    while(components[i] != NULL) {
        componentsId[i] = getComponentId(components[i], i);
        i++;
    }
    componentsId[i] = -1;
    return componentsId;
}

char** getCommandsArr(char* line){
    //return all commands for a current line
    char** commands = (char**)calloc(20, sizeof(char*));
    char* command = strtok(line, "=");
    int i = 0;
    while ((command = strtok(NULL, "|")) != NULL){
        commands[i++] = command;
    }
    return commands;
}

char** getProgramArgs(char* command){
    //get new program arguments - first argument is a path
    char** args = (char**)calloc(100, sizeof(char*));
    char* arg   = strtok(command, " ");
    int counter = 0;
    args[counter++] = arg;
    while ((arg = strtok(NULL, " ")) != NULL){
        args[counter++] = arg;
    }
    args[counter] = NULL;
    return args;
}

void getCommandsAndExecute(FILE* file){
    char** components = (char**)calloc(10, sizeof(char*));
    int command_end_flag = 0;
    char* current_line = (char*)calloc(100, sizeof(char));
    int line_ind = 0;

    while(fgets(current_line, 100*sizeof(char), file)){

        if(strcmp(current_line, "\n") == 0){
            command_end_flag = 1;
            continue;
        }

        if(command_end_flag == 0){
            printf("\n[SAVING COMPONENT]");
            char* line_copy = (char*) calloc(100, sizeof(char));
            strcpy(line_copy, current_line);
            components[line_ind] = line_copy;
            printf("Saved component: %s\n", components[line_ind]);
            line_ind++;
        }
        else{
            printf("\n[EXECUTING COMMAND]: %s", current_line);
            int component_counter = 0;
            int* componentsId = getComponentsId(current_line);
            printf("Components to execute: \n");
            while (componentsId[component_counter] != -1) {
                printf("%s", components[componentsId[component_counter]-1]);
                component_counter++;
            }

            int pipe_in[2];
            int pipe_out[2];
            if(pipe(pipe_out) != 0)
                exit(1);

            for(int i = 0; i < component_counter; i++) {
                char* current_component = components[componentsId[i]-1];
                printf("\nExecuting line %d:  %s \n", i, current_component);

                int commandCounter=0;
                char** commands = getCommandsArr(current_component);
                while (commands[commandCounter] != NULL) {
                    printf("com%d:  %s\n", commandCounter + 1, commands[commandCounter]);
                    commandCounter++;
                }

                for (int j = 0; j < commandCounter; j++) {
                    pid_t pid = fork();

                    if (pid == 0) {
                        //first command
                        if (j == 0 && i == 0) {
                            close(pipe_out[READ]);
                            //redirect
                            dup2(pipe_out[WRITE], STDOUT_FILENO);
                        }

                        //last command
                        else if (j == commandCounter - 1 && i == component_counter - 1) {
                            close(pipe_out[READ]);
                            close(pipe_out[WRITE]);
                            close(pipe_in[WRITE]);
                            dup2(pipe_in[READ], STDIN_FILENO);
                        }

                        //internal command
                        else {
                            close(pipe_in[WRITE]);
                            close(pipe_out[READ]);
                            dup2(pipe_in[READ], STDIN_FILENO);
                            dup2(pipe_out[WRITE], STDOUT_FILENO);
                        }
                        
                        char** args = getProgramArgs(commands[j]);

                        if (execvp(args[0], args) == -1) {
                            printf("exec failed\n");
                            exit(1);
                        }
                    } else {
                        //parent process moving pipes
                        close(pipe_in[WRITE]);
                        pipe_in[READ] = pipe_out[READ];
                        pipe_in[WRITE] = pipe_out[WRITE];
                        if (pipe(pipe_out) != 0) 
                            exit(1);   
                    }
                }
            }
            int status = 0;
            pid_t wpid;
            while ((wpid = wait(&status)) != -1);
            printf("\nALL CHILDREN TERMINATED\n");
        }
    }
}

int main(int argc, char* argv[]){
    if (argc != 2)
        exit(1);

    char* path = argv[1];
    FILE* file = fopen(path, "r");

    if (file == NULL)
        exit(1);

    getCommandsAndExecute(file);
    fclose(file);
}