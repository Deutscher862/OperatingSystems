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
    char* message = malloc(MAX_MSG_LEN*sizeof(char));
    strcpy(message, queue_name);

    if(mq_send(server_queue_desc, message, MAX_MSG_LEN, INIT) < 0)
        error_exit("cannot send message");

    unsigned int new_id;
    if(mq_receive(queue_desc, message, MAX_MSG_LEN, &new_id) < 0)
        error_exit("cannot receive message");

    client_id = new_id;
    printf("My new ID is: %d\n", client_id);
}

void handleLIST() {
    char* message = malloc(MAX_MSG_LEN*sizeof(char));
    message[0] = client_id;

    if(mq_send(server_queue_desc, message, MAX_MSG_LEN, LIST) < 0)
        error_exit("cannot send message");
    if(mq_receive(queue_desc, message, MAX_MSG_LEN, NULL) < 0)
        error_exit("cannot receive message");

    printf("%s\n", message);
}

void handleSTOP(){
    char* message = malloc(MAX_MSG_LEN*sizeof(char));
    message[0] = client_id;

    if(mq_send(server_queue_desc, message, MAX_MSG_LEN, STOP) < 0)
        error_exit("cannot send message");
    if(mq_close(server_queue_desc) < 0)
        error_exit("cannot close queue");
}

void enterChat(int receiver_id, mqd_t receiver_queue_id){
    char* command = NULL;
    ssize_t line = 0; // !!!!!!!!!!! zamiana na int?
    size_t len = 0;
    char* message = malloc(MAX_MSG_LEN*sizeof(char));
    while(1){
        printf("Enter message, DISCONNECT or STOP: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

        struct timespec* tspec = (struct timespec*)malloc(sizeof(struct timespec));
        unsigned int type;
        int disconnect = 0;

        while(mq_timedreceive(queue_desc, message, MAX_MSG_LEN, &type, tspec) >= 0) {
            if(type == STOP) {
                printf("STOP from server, quitting...\n");
                exit(0);
            } else if(type == DISCONNECT) {
                printf("Disconnecting...\n");
                disconnect = 1;
                break;
            } else {
                printf("[%d]: %s\n", receiver_id, message);
            }
        }

        if(disconnect) break;

        if(strcmp(command, "DISCONNECT") == 0) {
            message[0] = client_id;
            message[1] = receiver_id;
            if(mq_send(server_queue_desc, message, MAX_MSG_LEN, DISCONNECT) < 0) error_exit("cannot send message");
            break;
        } else if(strcmp(command, "") != 0) {
            strcpy(message, command);
            if(mq_send(receiver_queue_id, message, MAX_MSG_LEN, CONNECT) < 0) error_exit("cannot send message");
        }
    }
}

void handleCONNECT(int receiver_id) {
    //client initiating chat
    char* message = (char*)calloc(MAX_MSG_LEN, sizeof(char));
    message[0] = client_id;
    message[1] = receiver_id;

    if(mq_send(server_queue_desc, message, MAX_MSG_LEN, CONNECT) < 0)
        error_exit("cannot send message");

    if(mq_receive(queue_desc, message, MAX_MSG_LEN, NULL) < 0)
        error_exit("cannot receive message");

    char* receiver_queue_name = (char*)calloc(NAME_LEN, sizeof(char));
    strncpy(receiver_queue_name, message + 1, strlen(message) - 1);
    printf("other name %s\n", receiver_queue_name);
    mqd_t receiver_queue_desc = mq_open(receiver_queue_name, O_RDWR);
    if(receiver_queue_desc < 0)
        error_exit("cannot access other client queue");

    enterChat(receiver_id, receiver_queue_desc);
}

void getServerMessage() {
    //client checks if someone wants to connect with him
    char* message = malloc(MAX_MSG_LEN*sizeof(char));

    struct timespec* tspec = (struct timespec*)malloc(sizeof(struct timespec));
    unsigned int type;

    if(mq_timedreceive(queue_desc, message, MAX_MSG_LEN, &type, tspec) >= 0) {
        if(type == STOP) {
            printf("STOP from server, quitting...\n");
            exit(0);
        } else if(type == CONNECT) {
            printf("Connecting to client...\n");

            char* receiver_queue_name = malloc(NAME_LEN*sizeof(char));
            strncpy(receiver_queue_name, message + 1, strlen(message) - 1);
            printf("other name %s\n", receiver_queue_name);
            mqd_t receiver_queue_desc = mq_open(receiver_queue_name, O_RDWR);
            if(receiver_queue_desc < 0)
                error_exit("cannot access other client queue");

            enterChat((int) message[0], receiver_queue_desc);
        }
    }
}

int main(){
    srand(time(NULL));
    queue_name[0] = '/';
    for(int i = 1; i < NAME_LEN; i++) queue_name[i] = (rand() % ('Z' - 'A' + 1) + 'A');
    printf("My new queue name %s\n", queue_name);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_LEN - 1;
    attr.mq_curmsgs = 0;

    queue_desc = mq_open("/DUPA", O_RDONLY | O_CREAT | O_EXCL, &attr);
    if(queue_desc < 0)
        error_exit("cannot create queue");

    server_queue_desc = mq_open("/SERVER", O_RDWR);
    if(server_queue_desc < 0)
        error_exit("cannot access server queue");

    printf("New client desc: %d, server queue: %d\n", queue_desc, server_queue_desc);
    connectToServer();

    atexit(handleSTOP);

    char* command = NULL;
    ssize_t line;   // !!!!!!!!!!! zamiana na int?
    size_t len = 0;
    while(1){
        printf("Enter command: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

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