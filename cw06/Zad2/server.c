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

char* clients_queues[CLIENTS_LIMIT];
int connected_clients[CLIENTS_LIMIT];
mqd_t server_queue;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void handleINIT(char* message){
    printf("Received: [INIT]\n");

    //get new client ID
    int new_id = 0;
    while(new_id < CLIENTS_LIMIT && strcmp(clients_queues[new_id], "") != 0) 
        new_id++;
    if(new_id < CLIENTS_LIMIT) 
        new_id++;
    else{
        printf("Server limit exceeded\n");
        return;
    }

    //send new id client
    mqd_t client_queue_desc = mq_open(message, O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");

    connected_clients[new_id - 1] = 1;
    clients_queues[new_id - 1] = malloc(NAME_LEN*sizeof(char));
    strcpy(clients_queues[new_id - 1], message);

    char* respond = malloc(MAX_MSG_LEN*sizeof(char));
    if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, new_id) < 0)
        error_exit("cannot send message");
    if(mq_close(client_queue_desc) < 0)
        error_exit("cannot close queue");
}

void handleLIST(char* message){
    printf("Received: [LIST]\n");
    char* respond = malloc(MAX_MSG_LEN*sizeof(char));

    for(int i = 0; i < CLIENTS_LIMIT; i++) {
        if(strcmp(clients_queues[i], "") != 0){
            char* available;
            if(connected_clients[i] == 1)
                available = "available";
            else available = "not available";
            sprintf(respond + strlen(respond), "ID %d, client %s\n", i + 1, available);
        }
    }
    int client_id = (int) message[0];
    mqd_t client_queue_desc = mq_open(clients_queues[client_id - 1], O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");

    if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, LIST) < 0)
        error_exit("cannot send message");
    if(mq_close(client_queue_desc) < 0)
        error_exit("cannot close queue");
}

void handleCONNECT(char* message){
    printf("Received: [CONNECT]\n");
    int client_id = (int) message[0]; /////atoi ????
    int receiver_id = (int) message[1];

    mqd_t client_queue_desc = mq_open(clients_queues[client_id - 1], O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");

    char* respond = malloc(MAX_MSG_LEN*sizeof(char));
    respond[0] = receiver_id;
    strcat(respond, clients_queues[receiver_id - 1]);
    if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, CONNECT) < 0)
        error_exit("cannot send message");

    memset(respond, 0, strlen(respond));
    client_queue_desc = mq_open(clients_queues[receiver_id - 1], O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");
    respond[0] = client_id;
    strcat(respond, clients_queues[client_id - 1]);
    if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, CONNECT) < 0)
        error_exit("cannot send message");
    if(mq_close(client_queue_desc) < 0)
        error_exit("cannot close queue");

    connected_clients[client_id - 1] = 0;
    connected_clients[receiver_id - 1] = 0;
}

void handleDISCONNECT(char* message){
    printf("Received: [DISCONNECT]\n");
    int client_id = (int) message[0];
    int receiver_id = (int) message[1];

    char* respond = malloc(MAX_MSG_LEN*sizeof(char));
    mqd_t client_queue_desc = mq_open(clients_queues[receiver_id - 1], O_RDWR);
    if(client_queue_desc < 0)
        error_exit("cannot access client queue");
    if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, DISCONNECT) < 0)
        error_exit("cannot send message");
    if(mq_close(client_queue_desc) < 0)
        error_exit("cannot close queue");

    connected_clients[client_id - 1] = 1;
    connected_clients[receiver_id - 1] = 1;
}

void handleSTOP(char* message){
    printf("Received: [STOP]\n");
    int client_id = (int) message[0]; //yyyyyy po co to

    strcpy(clients_queues[client_id - 1], "");
    connected_clients[client_id - 1] = 0;
}

void handleMessage(char* message, int prio){
    if(prio == INIT){
        handleINIT(message);
    }
    else if(prio == LIST){
        handleLIST(message);
    }
    else if(prio == CONNECT){
        handleCONNECT(message);
    }
    else if(prio == DISCONNECT){
        handleDISCONNECT(message);
    }
    else if(prio == STOP){
        handleSTOP(message);
    }
    else printf("Invalid message type given\n");
}

void endWork(int signum) {
    char* respond = malloc(MAX_MSG_LEN*sizeof(char));
    //stop all clients
    for(int i = 0; i < CLIENTS_LIMIT; i++) {
        if(strcmp(clients_queues[i], "") != 0) {
            mqd_t client_queue_desc = mq_open(clients_queues[i], O_RDWR);
            if(client_queue_desc < 0)
                error_exit("cannot access client queue");
            if(mq_send(client_queue_desc, respond, MAX_MSG_LEN, STOP) < 0)
                error_exit("cannot send message");
            if(mq_receive(server_queue, respond, MAX_MSG_LEN, NULL) < 0)
                error_exit("cannot receive message");
            if(mq_close(client_queue_desc) < 0)
                error_exit("cannot close queue");
        }
    }
    //delete queue
    if(mq_close(server_queue) < 0)
        error_exit("cannot close queue");
    if(mq_unlink("/SERVER") < 0)
        error_exit("cannot delete queue");
    exit(0);
}

int main(int argc, char* argv[]){
    for(int i=0; i < CLIENTS_LIMIT; i++)
        clients_queues[i] = "";
    
    for(int i = 0 ; i < CLIENTS_LIMIT; i++)
        connected_clients[i] = 0;
    
    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = MAX_MSG_LEN - 1;
    attr.mq_curmsgs = 0;
    
    server_queue = mq_open("/SERVER", O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if(server_queue < 0) error_exit("cannot create queue");

    printf("New server queue ID: %d\n", server_queue);

    signal(SIGINT, endWork);

    char* message = malloc(MAX_MSG_LEN*sizeof(char));
    unsigned int prio;
    while(1){
        if(mq_receive(server_queue, message, MAX_MSG_LEN, &prio) < 0)
            error_exit("cannot receive message");
        handleMessage(message, prio);
    }
}