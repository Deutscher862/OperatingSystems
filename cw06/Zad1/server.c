#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>

#define MAX_CLIENTS 5

key_t clients_queues[MAX_CLIENTS];
int connected_clients[MAX_CLIENTS];
int server_queue_id;

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} m_type;

typedef struct Message {
    long m_type;
    char* message_type;
    char m_text[1024];
    key_t queue_key;
    int client_id;
    int receiver_id;
} Message;

const int MSG_SIZE = sizeof(Message) - sizeof(long);

void error_exit(char* msg) {
    printf("Error: %s\n", msg);
    printf("Errno: %d\n", errno);
    exit(EXIT_FAILURE);
}

void handleINIT(Message* message){
    printf("Received: [INIT]\n");

    int new_id = 0;
    while(new_id < MAX_CLIENTS && clients_queues[new_id] != -1) 
        new_id++;
    if(new_id < MAX_CLIENTS) 
        new_id++;
    else{
        printf("Server limit exceeded\n");
        return;
    }
    //czemu zwiększamy żeby pomniejszyć ID ??????????????
    Message* respond = (Message*)malloc(sizeof(Message));
    //respond->m_type = new_id !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    //respond->client_id = new_id;
    respond->m_type = new_id;

    int client_queue_id = msgget(message->queue_key, 0);
    if(client_queue_id < 0)
        error_exit("cannot access client queue");

    clients_queues[new_id - 1] = message->queue_key;
    connected_clients[new_id - 1] = 1;

    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
}

void handleLIST(Message* message){
    printf("Received: [INIT]\n");

    Message* respond = (Message*)malloc(sizeof(Message));
    strcpy(respond->m_text, "");

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients_queues[i] != -1){
            char* available;
            if(connected_clients[i] == 1)
                available = "available";
            else available = "not available";
            sprintf(respond->m_text + strlen(respond->m_text),"%d client is %s", i+1,  available);
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
    int other_client_id = message->receiver_id;

    respond->m_type = 5;
    respond->queue_key = clients_queues[other_client_id-1];
    int client_queue_id = msgget(clients_queues[client_id-1],0);

    if(client_queue_id < 0)
    error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
    
    respond->m_type = 5;
    respond->queue_key  =clients_queues[client_id-1];
    respond->client_id = client_id;
    client_queue_id = msgget(clients_queues[other_client_id-1],0);

    if(client_queue_id < 0)
        error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    connected_clients[client_id - 1] = 0;
    connected_clients[other_client_id - 1] = 0;
}

void handleDISCONNECT(Message* message){
    printf("Received: [DISCONNECT]\n");
    Message* respond = (Message*)malloc(sizeof(Message));
    int client_id = message->client_id;
    int other_client_id = message->receiver_id;

    respond->m_type = 2;

    int client_queue_id = msgget(clients_queues[other_client_id - 1], 0);
    if(client_queue_id < 0)
        error_exit("cannot access client queue");
    if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    connected_clients[client_id - 1] = 1;
    connected_clients[other_client_id - 1] = 1;
}

void handleSTOP(Message* message){
    printf("Received: [STOP]\n");
    int client_id = message->client_id;
    clients_queues[client_id - 1] = -1;
    connected_clients[client_id - 1] = 0;
}

void handleMessage(Message* message){
    int message_type = message->m_type;

    if(message_type == 3){
        handleINIT(message);
    }
    else if(message_type == 4){
        handleLIST(message);
    }
    else if(message_type == 5){
        handleCONNECT(message);
    }
    else if(message_type == 2){
        handleDISCONNECT(message);
    }
    else if(message_type == 2){
        handleSTOP(message);
    }
    else printf("Invalid message type given\n");
}

void deleteQueue(int signum) {
    Message* respond = (Message*)malloc(sizeof(Message));
    for(int i = 0; i < MAX_CLIENTS; i++) {
        key_t queue_key = clients_queues[i];
        if(queue_key != -1) {
            respond->m_type = STOP;
            // co znaczy to 0?????????????????????
            int client_queue_id = msgget(queue_key, 0);
            if(client_queue_id < 0)
                error_exit("cannot access client queue");
            //msgsnd - wysłanie komunikatu do kolejki
            if(msgsnd(client_queue_id, respond, MSG_SIZE, 0) < 0)
                error_exit("cannot send message");
            //msgrcv - odebranie komunikatu z kolejki
            if(msgrcv(server_queue_id, respond, MSG_SIZE, STOP, 0) < 0)
                error_exit("cannot receive message");
        }
    }
    //msgctl - modyfikowanie oraz odczyt rozmaitych właściwości kolejki
    //delete queue
    msgctl(server_queue_id, IPC_RMID, NULL);
    exit(0);
}

int main(int argc, char* argv[]){
    for(int i=0; i < MAX_CLIENTS; i++)
        clients_queues[i] = -1;
    
    for(int i = 0 ; i < MAX_CLIENTS; i++)
        connected_clients[i] = 0;
    //ftok = generowanie wartości kluczy
    //getenv = zmienna środowiskowa
    key_t queue_key = ftok(getenv("HOME"), 1);
    printf("Generater key: %d\n", queue_key);
    //uzyskanie identyfikatora kolejki komunikatów używanego przez pozostałe funkcje,
    //ipc_creat - stworzenie kolejki
    server_queue_id = msgget(queue_key, IPC_CREAT | 0666);

    printf("Queue ID: %d\n", server_queue_id);

    signal(SIGINT, deleteQueue);

    Message* message = malloc(sizeof(Message));
    while(1){
        if(msgrcv(server_queue_id, message, MSG_SIZE, -6, 0) < 0)
            error_exit("cannot receive message");
        handleMessage(message);
    }
}