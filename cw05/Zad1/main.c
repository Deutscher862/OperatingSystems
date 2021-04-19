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

char** getComponents(char** components_arr, char* command_line){
    //return array of components for current commandline
    char** components = (char**)calloc(20, sizeof(char*));
    char* current_component = strtok (command_line,"|");
    int counter = 0;
    while (current_component != NULL){
        int component_id = getComponentId(current_component, counter);
        char* comp = components_arr[component_id-1];
        comp[strcspn(comp, "\n")] = 0;
        components[counter] = comp;
        counter++;
        current_component = strtok(NULL, "|");
    }
    components[counter] = NULL;
    return components;
}

char** splitComponent(char* component){
    //split set of components into singular ones
    char** commands = (char**)calloc(20, sizeof(char*));
    char* current_command = strtok (component,"|");
    int counter = 0;
    while (current_command != NULL){
        char* comp = current_command;

        comp[strcspn(comp, "\n")] = 0;
        commands[counter] = comp;
        counter++;
        current_command = strtok(NULL, "|");
    }
    commands[counter] = NULL;
    return commands;
}

char** splitCommand(char* command){
    //split one command into path and arguments
    char** arguments = (char**)calloc(20, sizeof(char*));
    char* arg = strtok (command," ");
    int counter = 0;
    while (arg != NULL){
        char* comp = arg;
        comp[strcspn(comp, "\n")] = 0;
        arguments[counter] = comp;
        counter++;
        arg = strtok(NULL, " ");
    }
    arguments[counter] = NULL;
    return arguments;
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

        //saving components
        if(command_end_flag == 0){
            printf("\n[SAVING COMPONENT]");
            char* line_copy = (char*) calloc(100, sizeof(char));
            strncpy(line_copy, current_line+12, strlen(current_line));
            components[line_ind] = line_copy;
            printf("Saved component: %s\n", components[line_ind]);
            line_ind++;
        }
        else{
            //executing commands
            printf("\n[EXECUTING COMMAND]: %s\n", current_line);
            char** executing_components = getComponents(components, current_line);
            int components_counter = 0;
            printf("Componets to execute:\n");
            
            while(executing_components[components_counter] != NULL)
                printf("%s\n", executing_components[components_counter++]);
            
            for(int i = 0; i < components_counter; i++){
                char* current_component = executing_components[i];
                printf("\nCurrently executing component: %s\n", current_component);

                char** commands = splitComponent(current_component);
                int command_counter = 0;
                printf("Commands to execute:\n");
                while(commands[command_counter] != NULL)
                    printf("%s\n", commands[command_counter++]);

                int pipes[32][2];

                for (int j = 0; j < command_counter; j++){
                    pid_t pid = fork();

                    if (pid == 0){
                        if(j > 0){
                            dup2(pipes[j - 1][0], STDIN_FILENO);
                        }
                        if(j + 1 < command_counter){
                            dup2(pipes[j][1], STDOUT_FILENO);
                        }
                        for(int k = 0; k < command_counter - 1; k++){
                            close(pipes[k][0]);
                            close(pipes[k][1]);
                        }
                        
                        char** args = splitCommand(commands[j]);
                        printf("\nARGUMENTS: \n");
                        for(int k = 0; args[k] != NULL; k++){
                            printf("arg%d %s\n", k, args[k]);
                        }
                        
                        if(execvp(args[0], args) == -1){
                            printf("exec failed\n");
                            exit(1);
                        }
                    }
                    for (int k = 0; k < command_counter - 1; ++k){
                        close(pipes[k][0]);
                        close(pipes[k][1]);
                    }
                }
            }
            int status = 0;
            pid_t pid;
            while ((pid = wait(&status)) != -1);
            printf("\n[COMMANDS EXECUTED]\n");
        }
    }
}

int main(int argc, char* argv[]){
    if(argc != 2)
        exit(1);

    char* path = argv[1];
    FILE* file = fopen(path, "r");

    if(file == NULL)
        exit(1);

    getCommandsAndExecute(file);
    fclose(file);
}