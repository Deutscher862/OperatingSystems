#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <mqueue.h>
#include <errno.h>
#include "config.h"

char queue_name[NAME_LEN];

mqd_t queue_desc;
mqd_t server_queue_desc;
int client_id;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void connectToServer(){
    //initiating server connection
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %s", INIT, queue_name);
    if(mq_send(server_queue_desc, message, strlen(message), INIT) < 0)
        error_exit("cannot send message");
    
    //get new id from server
    unsigned int type;
    char server_respond[MAX_MESSAGE_SIZE];
    if(mq_receive(queue_desc, server_respond, MAX_MESSAGE_SIZE, &type) < 0)
        error_exit("cannot receive message");
        
    sscanf(server_respond, "%d %d", &type, &client_id);
    printf("My new ID is: %d\n", client_id);
}

void handleLIST() {
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %d", LIST, client_id);
    if(mq_send(server_queue_desc, message, strlen(message), LIST) < 0)
        error_exit("cannot send message");
}

void handleSTOP(){
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %d", STOP, client_id);
    if(mq_send(server_queue_desc, message, strlen(message), STOP) < 0)
        error_exit("cannot send message");
    mq_close(server_queue_desc);
    mq_close(queue_desc);
    mq_unlink(queue_name);
}

void enterChat(int receiver_id, mqd_t receiver_queue_desc){
    char* command = NULL;
    size_t len = 0;
    ssize_t line = 0;
    char* message = malloc(MAX_MESSAGE_SIZE*sizeof(char));
    while(1){
        printf("Enter message, DISCONNECT or STOP: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

        struct timespec* tspec = (struct timespec*)malloc(sizeof(struct timespec));
        unsigned int type;
        int disconnect = 0;
        while(mq_timedreceive(queue_desc, message, MAX_MESSAGE_SIZE, &type, tspec) >= 0) {
            if(type == STOP) {
                exit(0);
            } else if(type == DISCONNECT) {
                printf("Disconnecting...\n");
                disconnect = 1;
                break;
            } else {
                printf("[%d said]: %s\n", receiver_id, message);
            }
        }
        if(disconnect == 1) break;

        if(strcmp(command, "DISCONNECT") == 0) {
            char dsc_message[MAX_MESSAGE_SIZE];
            sprintf(dsc_message, "%d %d %d", DISCONNECT, client_id, receiver_id);
            if(mq_send(server_queue_desc, dsc_message, strlen(dsc_message), DISCONNECT) < 0)
                error_exit("cannot send message");
            break;
        } else if(strcmp(command, "") != 0) {
            char new_message[MAX_MESSAGE_SIZE];
            strcpy(new_message, command);
            if(mq_send(receiver_queue_desc, new_message, strlen(new_message), CONNECT) < 0)
                error_exit("cannot send message");
        }
    }
}

void handleCONNECT(int receiver_id){
    if(client_id == receiver_id){
        printf("You cannot connect to yourself!\n");
        return;
    }

    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %d %d", CONNECT, client_id, receiver_id);
    if(mq_send(server_queue_desc, message, strlen(message), CONNECT) < 0)
        error_exit("cannot send message");

    char* server_respond = malloc(MAX_MESSAGE_SIZE*sizeof(char));
    int send_receiver_id;
    unsigned int type;
    if(mq_receive(queue_desc, server_respond, MAX_MESSAGE_SIZE, &type) < 0)
        error_exit("cannot receive message");

    char receiver_queue_name[NAME_LEN];
    sscanf(server_respond, "%d %d %s", &type, &send_receiver_id, receiver_queue_name);

    mqd_t receiver_queue_desc = mq_open(receiver_queue_name, O_RDWR);
    if(receiver_queue_desc < 0)
        error_exit("cannot access other client queue");

    enterChat(send_receiver_id, receiver_queue_desc);
}

void getServerMessage() {
    char* message = (char*)calloc(MAX_MESSAGE_SIZE, sizeof(char));

    struct timespec* tspec = (struct timespec*)malloc(sizeof(struct timespec));
    unsigned int type;
    if(mq_timedreceive(queue_desc, message, MAX_MESSAGE_SIZE, &type, tspec) >= 0) {
        printf("DUPSKO");
        if(type == STOP) {
            exit(0);
        } else if(type == CONNECT) {
            int receiver_id;
            unsigned int type;
            if(mq_receive(queue_desc, message, MAX_MESSAGE_SIZE, &type) < 0)
                error_exit("cannot receive message");

            char receiver_queue_name[NAME_LEN];
            sscanf(message, "%d %d %s", &type, &receiver_id, receiver_queue_name);

            mqd_t receiver_queue_desc = mq_open(receiver_queue_name, O_RDWR);
            if(receiver_queue_desc < 0)
                error_exit("cannot access other client queue");

            enterChat(receiver_id, receiver_queue_desc);
        }
    }
}

int main(){
    atexit(handleSTOP);
    srand(time(NULL));
    queue_name[0] = '/';
    for(int i = 1; i < NAME_LEN; i++) queue_name[i] = (rand() % ('Z' - 'A' + 1) + 'A');
    printf("My new queue name: %s\n", queue_name);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_SIZE - 1;
    attr.mq_curmsgs = 0;

    server_queue_desc = mq_open("/SERVER", O_RDWR);
    if(server_queue_desc < 0)
        error_exit("cannot access server queue");

    queue_desc = mq_open(queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if(queue_desc < 0)
        error_exit("cannot create queue");

    connectToServer();

    char* command = NULL;
    ssize_t line;
    size_t len = 0;
    while(1){
        printf("Enter command: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

        if(strcmp(command, "") == 0)
            continue;

        getServerMessage();

        if(strcmp(command, "") == 0)
            continue;

        char* input = strtok(command, " ");
        if(strcmp(input, "LIST") == 0){
            handleLIST();
        } else if(strcmp(input, "CONNECT") == 0){
            input = strtok(NULL, " ");
            handleCONNECT(atoi(input));
        } else if(strcmp(input, "STOP") == 0){
            exit(0);
        } else printf("Unrecognized command: %s\n", command);
    }
    return 0;
}