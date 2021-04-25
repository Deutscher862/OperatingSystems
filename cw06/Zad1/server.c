#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include "config.h"

#define CLIENTS_LIMIT 10

key_t clients_queues[CLIENTS_LIMIT];
int connected_clients[CLIENTS_LIMIT];
int server_queue_id;

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void handleINIT(Message* message){
    printf("Received: [INIT]\n");

    //get new client ID
    int new_id = 0;
    while(new_id < CLIENTS_LIMIT && clients_queues[new_id] != -1) 
        new_id++;
    if(new_id < CLIENTS_LIMIT) 
        new_id++;
    else{
        printf("Server limit exceeded\n");
        return;
    }

    //send new id client
    Message* respond = (Message*)malloc(sizeof(Message));
    respond->m_type = INIT;
    respond->client_id = new_id;

    int client_queue_id = msgget(message->queue_key, 0);
    if(client_queue_id < 0)
        error_exit("cannot access client queue");

    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    //update server queue
    clients_queues[new_id - 1] = message->queue_key;
    connected_clients[new_id - 1] = 1;
}

void handleLIST(Message* message){
    printf("Received: [INIT]\n");

    Message* respond = (Message*)malloc(sizeof(Message));
    strcpy(respond->m_text, "");

    for(int i = 0; i < CLIENTS_LIMIT; i++){
        if(clients_queues[i] != -1){
            char* available;
            if(connected_clients[i] == 1)
                available = "available";
            else available = "not available";
            sprintf(respond->m_text + strlen(respond->m_text),"%d client is %s\n", i+1,  available);
        }
    }
    int client_id = message->client_id;
    int client_queue_id = msgget(clients_queues[client_id - 1], 0);
    if(client_queue_id < 0)
        error_exit("cannot access client queue");

    respond->m_type = client_id;
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0) error_exit("cannot send message");
}

void handleCONNECT(Message* message){
    printf("Received: [CONNECT]\n");
    Message* respond = (Message*)malloc(sizeof(Message));
    int client_id = message->client_id;
    int receiver_id = message->receiver_id;
    printf("Connecting client %d with %d\n", client_id, receiver_id);

    //inform both clients about connecting
    respond->m_type = CONNECT;
    respond->queue_key = clients_queues[receiver_id-1];
    int client_queue_id = msgget(clients_queues[client_id-1],0);
    
    if(client_queue_id < 0)
        error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
    
    respond->queue_key = clients_queues[client_id-1];
    respond->client_id = client_id;
    client_queue_id = msgget(clients_queues[receiver_id-1],0);

    if(client_queue_id < 0)
        error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
   
    //if client connects with another they become unavailable
    connected_clients[client_id - 1] = 0;
    connected_clients[receiver_id - 1] = 0;
}

void handleDISCONNECT(Message* message){
    printf("Received: [DISCONNECT]\n");
    Message* respond = (Message*)malloc(sizeof(Message));
    int client_id = message->client_id;
    int receiver_id = message->receiver_id;

    respond->m_type = DISCONNECT;
    //inform the other client that this one has been disconnected
    int client_queue_id = msgget(clients_queues[receiver_id - 1], 0);
    if(client_queue_id < 0)
        error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    connected_clients[client_id - 1] = 1;
    connected_clients[receiver_id - 1] = 1;
}

void handleSTOP(Message* message){
    printf("Received: [STOP]\n");
    int client_id = message->client_id;
    clients_queues[client_id - 1] = -1;
    connected_clients[client_id - 1] = 0;
}

void handleMessage(Message* message){
    int message_type = message->m_type;

    if(message_type == INIT){
        handleINIT(message);
    }
    else if(message_type == LIST){
        handleLIST(message);
    }
    else if(message_type == CONNECT){
        handleCONNECT(message);
    }
    else if(message_type == DISCONNECT){
        handleDISCONNECT(message);
    }
    else if(message_type == STOP){
        handleSTOP(message);
    }
    else printf("Invalid message type given\n");
}

void endWork(int signum) {
    Message* respond = (Message*)malloc(sizeof(Message));
    //stop all clients
    for(int i = 0; i < CLIENTS_LIMIT; i++) {
        key_t queue_key = clients_queues[i];
        if(queue_key != -1) {
            respond->m_type = STOP;
            int client_queue_id = msgget(queue_key, 0);

            if(client_queue_id < 0)
                error_exit("cannot access client queue");

            if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
                error_exit("cannot send message");

            if(msgrcv(server_queue_id, respond, MSG_SIZE, STOP, 0) < 0)
                error_exit("cannot receive message");
        }
    }
    //delete queue
    msgctl(server_queue_id, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char* argv[]){
    for(int i=0; i < CLIENTS_LIMIT; i++)
        clients_queues[i] = -1;
    
    for(int i = 0 ; i < CLIENTS_LIMIT; i++)
        connected_clients[i] = 0;
    
    //generating key
    key_t queue_key = ftok(getenv("HOME"), 1);
    printf("Generater key: %d\n", queue_key);

    //creating queue
    server_queue_id = msgget(queue_key, IPC_CREAT | 0666);

    printf("Queue ID: %d\n", server_queue_id);

    signal(SIGINT, endWork);

    Message* message = malloc(sizeof(Message));
    while(1){
        if(msgrcv(server_queue_id, message, MSG_SIZE, -6, 0) < 0)
            error_exit("cannot receive message");
        handleMessage(message);
    }
}