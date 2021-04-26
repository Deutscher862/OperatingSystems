#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "config.h"

#define CLIENTS_LIMIT 10

char* clients_names[CLIENTS_LIMIT];
int clients_queues[CLIENTS_LIMIT];
int connected_clients[CLIENTS_LIMIT];
mqd_t server_queue;
/*
int get_queue(char *name) { return mq_open(name, O_WRONLY); }

void send_message(mqd_t desc, char *msgPointer, int type) { mq_send(desc, msgPointer, strlen(msgPointer), type); }

void receive_message(mqd_t desc, char *msgPointer, int *typePointer) { mq_receive(desc, msgPointer, MAX_MESSAGE_SIZE, typePointer); }
*/

void error_exit(char* message) {
    printf("Error: %s\n", message);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void handleINIT(char* message){
    printf("Received: [INIT]\n");

    //get new client ID
    int new_id = 0;
    while(new_id < CLIENTS_LIMIT && strcmp(clients_names[new_id], "") != 0) 
        new_id++;
    if(new_id < CLIENTS_LIMIT) 
        new_id++;
    else{
        printf("Server limit exceeded\n");
        return;
    }   

    int type;
    char* client_name = malloc(MAX_MESSAGE_SIZE * sizeof(char));
    sscanf(message, "%d %s", &type, client_name);

    //send new id client
    mqd_t client_queue_desc = mq_open(client_name, O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");

    connected_clients[new_id - 1] = 1;
    clients_names[new_id - 1] = malloc(NAME_LEN*sizeof(char));
    strcpy(clients_names[new_id - 1], client_name);
    clients_queues[new_id - 1] = client_queue_desc;

    char respond[MAX_MESSAGE_SIZE];
    sprintf(respond, "%d %d", INIT, new_id);
    if(mq_send(client_queue_desc, respond, strlen(respond), INIT) < 0)
        error_exit("cannot send message");
}

void handleLIST(char* message){
    printf("Received: [LIST]\n");
    int type, client_id;
    sscanf(message, "%d %d", &type, &client_id);

    for(int i = 0; i < CLIENTS_LIMIT; i++) {
        if(strcmp(clients_names[i], "") != 0){
            char* available;
            if(connected_clients[i] == 1)
                available = "available";
            else available = "not available";
            printf("ID %d is client %s\n", i + 1, available);
        }
    }
}

void handleCONNECT(char* message){
    printf("Received: [CONNECT]\n");

    int type, client_id, receiver_id;
    sscanf(message, "%d %d %d", &type, &client_id, &receiver_id);
    printf("Connecting client %d with %d\n", client_id, receiver_id);

    if(connected_clients[client_id] == 0){
        printf("Client %d is not available", client_id);
        return;
    }
    else if(connected_clients[receiver_id] == 0){
        printf("Client %d is not available", receiver_id);
        return;
    }

    //inform both clients about connecting
    char respond1[MAX_MESSAGE_SIZE];
    char respond2[MAX_MESSAGE_SIZE];

    sprintf(respond1, "%d %s", CONNECT, clients_names[receiver_id -1]);
    sprintf(respond2, "%d %s", CONNECT, clients_names[client_id -1]);

    if(mq_send(clients_queues[receiver_id - 1], respond1, strlen(respond1), CONNECT))
        error_exit("cannot send message");
    
    if(mq_send(clients_queues[client_id - 1], respond2, strlen(respond2), CONNECT))
        error_exit("cannot send message");
   
    //if client connects with another they become unavailable
    connected_clients[client_id - 1] = 0;
    connected_clients[receiver_id - 1] = 0;
}

void handleDISCONNECT(char* message){
    printf("Received: [DISCONNECT]\n");
    int type, client_id;
    sscanf(message, "%d %d", &type, &client_id);

    connected_clients[client_id - 1] = 1;
    printf("Server -- client with id %d left chat\n", client_id);
}

void handleSTOP(char* message){
    printf("Received: [STOP]\n");
    int type, client_id;
    sscanf(message, "%d %d", &type, &client_id);

    strcpy(clients_names[client_id - 1], "");
    connected_clients[client_id - 1] = 0;
    clients_queues[client_id - 1] = 0;
}

void handleMessage(char* message, int type){
    if(type == INIT){
        handleINIT(message);
    }
    else if(type == LIST){
        handleLIST(message);
    }
    else if(type == CONNECT){
        handleCONNECT(message);
    }
    else if(type == DISCONNECT){
        handleDISCONNECT(message);
    }
    else if(type == STOP){
        handleSTOP(message);
    }
    else printf("Invalid message type given\n");
}

void endWork(){
    char* respond = malloc(MAX_MESSAGE_SIZE*sizeof(char));
    //stop all clients
    for(int i = 0; i < CLIENTS_LIMIT; i++) {
        if(strcmp(clients_names[i], "") != 0) {
            mqd_t client_queue_desc = mq_open(clients_names[i], O_RDWR);
            if(client_queue_desc < 0)
                error_exit("cannot access client queue");
            if(mq_send(client_queue_desc, respond, MAX_MESSAGE_SIZE, STOP) < 0)
                error_exit("cannot send message");
            if(mq_receive(server_queue, respond, MAX_MESSAGE_SIZE, NULL) < 0)
                error_exit("cannot receive message");
            if(mq_close(client_queue_desc) < 0)
                error_exit("cannot close queue");
        }
    }
    //delete queue
    mq_close(server_queue);
    mq_unlink("/SERVER");
    exit(0);
}

void exitSignal(int signum){
    endWork();
}

int main(int argc, char* argv[]){
    for(int i=0; i < CLIENTS_LIMIT; i++)
        clients_names[i] = "";
    
    for(int i = 0 ; i < CLIENTS_LIMIT; i++)
        connected_clients[i] = 0;
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_SIZE - 1;
    attr.mq_curmsgs = 0;

    server_queue = mq_open("/SERVER", O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);

    if(server_queue < 0) error_exit("cannot create queue");

    printf("New server queue ID: %d\nWaiting for commands...\n", server_queue);

    signal(SIGINT, exitSignal);
    atexit(endWork);

    char* message = malloc(MAX_MESSAGE_SIZE*sizeof(char));
    unsigned int type;
    while(1){
        //receive_message(server_queue, message, &type);
        mq_receive(server_queue, message, MAX_MESSAGE_SIZE, &type);
        handleMessage(message, type);
    }
    free(message);
}