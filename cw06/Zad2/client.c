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

#define MESSAGE 6

char queue_name[NAME_LEN];

mqd_t queue_desc;
mqd_t server_queue_desc;
int client_id;
int receiver_id = -1;
/*
int get_queue(char *name) { return mq_open(name, O_WRONLY); }

void send_message(mqd_t desc, char *msgPointer, int type) { mq_send(desc, msgPointer, strlen(msgPointer), type); }

void receive_message(mqd_t desc, char *msgPointer, int *typePointer) { mq_receive(desc, msgPointer, MAX_MESSAGE_SIZE, typePointer); }
*/

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void connectToServer(){
    //initiating server connection
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %s", INIT, queue_name);
    printf("Sending %s\n", message);
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

void handleDISCONNECT(){
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %d", DISCONNECT, client_id);
    if(mq_send(server_queue_desc, message, strlen(message), DISCONNECT) < 0)
        error_exit("cannot send message");

    if (receiver_id != -1)
    {
        char inform_receiver[MAX_MESSAGE_SIZE];
        sprintf(message, "%d %d", DISCONNECT, client_id);
        if(mq_send(receiver_id, inform_receiver, strlen(inform_receiver), DISCONNECT))
            error_exit("cannot send message");
        mq_close(receiver_id);
        receiver_id = -1;
    }
}

void handleCONNECT(int receiver_id){
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %d %d", CONNECT, client_id, receiver_id);
    if(mq_send(server_queue_desc, message, strlen(message), CONNECT) < 0)
        error_exit("cannot send message");
}

void sendMessage(char *message_to_send)
{
    char message[MAX_MESSAGE_SIZE];
    sprintf(message, "%d %s", MESSAGE, message_to_send);
    if(mq_send(receiver_id, message, strlen(message), MESSAGE))
        error_exit("cannot send message");

    printf("------------------------------------------------\n");
    printf("M:          \t%s\n", message);
    printf("------------------------------------------------\n");
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

    printf("New client desc: %d, server queue: %d\n", queue_desc, server_queue_desc);
    connectToServer();

    char* command = NULL;
    ssize_t line;   // !!!!!!!!!!! zamiana na int?
    size_t len = 0;
    while(1){
        printf("Enter command: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

        if(strcmp(command, "") == 0)
            continue;

        char* input = strtok(command, " ");
        if(strcmp(input, "STOP") == 0){
            handleSTOP();
        }
        else if(strcmp(input, "LIST") == 0){
            handleLIST();
        }
        else if(strcmp(input, "DISCONNECT") == 0){
            handleDISCONNECT();
        }
        else if(strcmp(input, "CONNECT") == 0){
            input = strtok(NULL, " ");
            handleCONNECT(atoi(input));
        }
        else if(strcmp(input, "MESSAGE") == 0)
        {
            char message[MAX_MESSAGE_SIZE];
            scanf("%s", message);

            if(receiver_id == -1)
                printf("Client -- unable to send message\n");
            else{
                sendMessage(message);
            }
        } else printf("Unrecognized command: %s\n", command);
    }
    return 0;
}