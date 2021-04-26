#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <errno.h>
#include "config.h"

key_t queue_key;
int client_queue_id;
int server_queue_id;
int client_id;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void connectToServer(){
    //initiating server connection
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = INIT;
    message->queue_key = queue_key;

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    Message* server_respond = (Message*)malloc(sizeof(Message));
    if(msgrcv(client_queue_id, server_respond, MSG_SIZE, 0, 0) < 0)
        error_exit("cannot receive message");

    client_id = server_respond->client_id;
    printf("My new ID is: %d\n", client_id);
}

void handleLIST() {
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = LIST;
    message->client_id = client_id;
    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
}

void handleSTOP(){
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = STOP;
    message->client_id = client_id;

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
    if(msgctl(client_queue_id, IPC_RMID, NULL) < 0)
        error_exit("cannot delete queue");

    msgctl(client_queue_id, IPC_RMID, NULL);
}

void disconnect(int receiver_id){
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = DISCONNECT;
    message->client_id = client_id;
    message->receiver_id = receiver_id;
    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
}

void enterChat(int receiver_id, int receiver_queue_id){
    char* command = NULL;
    ssize_t line = 0; // !!!!!!!!!!! zamiana na int?
    size_t len = 0;
    Message* message = (Message*)malloc(sizeof(Message));
    while(1){
        printf("Enter message, DISCONNECT or STOP: ");
        line = getline(&command, &len, stdin);
        command[line - 1] = '\0';

        //stopping has the highest priority
        if(msgrcv(client_queue_id, message, MSG_SIZE, STOP, IPC_NOWAIT) >= 0)
            exit(0);

        //if the other client has disconnected, end chat
        if(msgrcv(client_queue_id, message, MSG_SIZE, DISCONNECT, IPC_NOWAIT) >= 0){
            printf("Quitting chat\n");
            break;
        }

        while(msgrcv(client_queue_id, message, MSG_SIZE, 0, IPC_NOWAIT) >= 0)
            printf("[%d said]: %s\n", receiver_id, message->m_text);

        if(strcmp(command, "DISCONNECT") == 0){
            disconnect(receiver_id);
            break;
        }
        else if(strcmp(command, "STOP") == 0){
            disconnect(receiver_id);
            exit(0);
        } else if(strcmp(command, "") != 0) {
            message->m_type = CONNECT;
            strcpy(message->m_text, command);
            if(msgsnd(receiver_queue_id, message, MSG_SIZE, 0) < 0)
                error_exit("cannot send message");
        }
    }
}

void handleCONNECT(int receiver_id) {
    //client initiating chat
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = CONNECT;
    message->client_id = client_id;
    message->receiver_id = receiver_id;

    if(client_id == receiver_id){
        printf("You cannot connect to yourself!\n");
        return;
    }

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    Message* server_respond = (Message*)malloc(sizeof(Message));
    if(msgrcv(client_queue_id, server_respond, MSG_SIZE, 0, 0) < 0)
        error_exit("cannot receive message");

    key_t receiver_queue_key = server_respond->queue_key;
    int receiver_queue_id = msgget(receiver_queue_key, 0);
    if(receiver_queue_id < 0)
        error_exit("cannot access other client queue");

    enterChat(receiver_id, receiver_queue_id);
}

void getServerMessage() {
    //client checks if someone wants to connect with him
    Message* message = (Message*)malloc(sizeof(Message));

    if(msgrcv(client_queue_id, message, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
        if(message->m_type == STOP) {
            printf("STOP from server, quitting...\n");
            exit(0);
        } else if(message->m_type == CONNECT) {
            printf("Connecting to client %d...\n", message->client_id);
            int receiver_queue_id = msgget(message->queue_key, 0);
            if(receiver_queue_id < 1)
                error_exit("cannot access other client queue");
            enterChat(message->client_id, receiver_queue_id);
        }
    }
}

int main(){
    srand(time(NULL));
    queue_key = ftok(getenv("HOME"), (rand() % 255 + 1));
    
    printf("My new queue key: %d\n", queue_key);    // !!!!!!
    client_queue_id = msgget(queue_key, IPC_CREAT | 0666);
    if(client_queue_id < 0)
        error_exit("cannot create queue");
    printf("My new queue ID: %d\n", client_queue_id); // !!!!!!

    key_t server_key = ftok(getenv("HOME"), 1);
    server_queue_id = msgget(server_key, 0);
    if(server_queue_id < 0)
        error_exit("cannot access server queue");
    printf("Server queue ID: %d\n", server_queue_id);   // !!!!!!

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