#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_output(FILE* file){
    char line[100];
    while(fgets(line, 100, file) != NULL){
        printf("%s\n", line);
    }
}

void print_ordered_by(char* mode){
    FILE* file;
    char* command;

    if (strcmp(mode,"date") == 0) {
        command = "echo | mail -f | tail +2 | head -n -1 | tac";
    }
    else if (strcmp(mode, "sender") == 0){
        command = "echo | mail -f | tail +2 | head -n -1 | sort -k 2";
    }
    else
        exit(1);

    file = popen(command, "r");

    if (file == NULL)
        exit(1);

    print_output(file);
    pclose(file);
}

void send_email(char* address, char* subject, char* content){
    FILE* file;
    char command[100];

    snprintf(command, sizeof(command), "echo %s | mail -s %s %s", content, subject, address);
    printf("%s\n", command);
    file = popen(command, "r");

    if (file == NULL)
        exit(1);
    print_output(file);
    pclose(file);
}

int main(int argc, char* argv[]){
    if (argc == 2){
        print_ordered_by(argv[1]);
    }
    else if(argc == 4){
        send_email(argv[1], argv[2], argv[3]);
    }
    else{
        printf("Wrong number of arguments!");
        exit(1);
    }
}