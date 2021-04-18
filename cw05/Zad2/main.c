#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc == 2){
        if (strcmp(argv[1], "date") == 0){
            FILE* mail = popen("mail | sort -M", "w");
            pclose(mail);
        }
        else if(strcmp(argv[1], "sender") == 0){
            FILE* mail = popen("mail | sort -k 3", "w");
            pclose(mail);
        }
    }
    else if (argc == 4){
        char* adress = argv[1];
        char* subject = argv[2];
        char* content = argv[3];
        char command[100];
        sprintf(command, "mail -s %s %s", subject, adress);
        
        FILE* mail = popen(command, "w");
        fputs(content, mail);
        pclose(mail);
        printf("Mail sent succesfully\n");
    }
    return 0;
}