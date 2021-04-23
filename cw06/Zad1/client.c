#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS 5

key_t queue_key;
int client_queue_id;
int server_queue_id;
int client_id;

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

void connectToServer(){
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = 3;
    message->queue_key = queue_key;

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    Message* server_respond = (Message*)malloc(sizeof(Message));
    if(msgrcv(client_queue_id, server_respond, MSG_SIZE, 0, 0) < 0)
        error_exit("cannot receive message");

    client_id = server_respond->m_type;
}

void handleLIST() {
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = 4;
    message->client_id = client_id;
    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    Message* server_respond = (Message*)malloc(sizeof(Message));
    if(msgrcv(client_queue_id, server_respond, MSG_SIZE, 0, 0) < 0)
        error_exit("cannot receive message");
    printf("%s\n", server_respond->m_text);
}

void handleSTOP(){
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = 1;
    message->client_id = client_id;

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");
    if(msgctl(client_queue_id, IPC_RMID, NULL) < 0)
        error_exit("cannot delete queue");

    msgctl(client_queue_id, IPC_RMID, NULL);
    exit(0);
}

void enterChat(int other_id, int other_queue_id) {
    char* command = NULL;
    size_t len = 0;
    ssize_t read = 0;
    Message* message = (Message*)malloc(sizeof(Message));
    while(1) {
        printf("Enter message or DISCONNECT: ");
        read = getline(&command, &len, stdin);
        command[read - 1] = '\0';

        if(msgrcv(client_queue_id, message, MSG_SIZE, 1, IPC_NOWAIT) >= 0) {
            printf("STOP from server, quitting...\n");
            handleSTOP();
        }

        if(msgrcv(client_queue_id, message, MSG_SIZE, 2, IPC_NOWAIT) >= 0) {
            printf("Disconnecting...\n");
            break;
        }

        while(msgrcv(client_queue_id, message, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
            printf("[%d]: %s\n", other_id, message->m_text);
        }

        if(strcmp(command, "DISCONNECT") == 0) {
            message->m_type = 2;
            message->client_id = client_id;
            message->receiver_id = other_id;
            if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
                error_exit("cannot send message");
            break;
        } else if(strcmp(command, "") != 0) {
            message->m_type = 5;
            strcpy(message->m_text, command);
            if(msgsnd(other_queue_id, message, MSG_SIZE, 0) < 0)
                error_exit("cannot send message");
        }
    }
}

void handleCONNECT(int id) {
    Message* message = (Message*)malloc(sizeof(Message));
    message->m_type = 5;
    message->client_id = client_id;
    message->receiver_id = id;

    if(msgsnd(server_queue_id, message, MSG_SIZE, 0) < 0)
        error_exit("cannot send message");

    Message* server_respond = (Message*)malloc(sizeof(Message));
    if(msgrcv(client_queue_id, server_respond, MSG_SIZE, 0, 0) < 0)
        error_exit("cannot receive message");

    key_t other_queue_key = server_respond->queue_key;
    int other_queue_id = msgget(other_queue_key, 0);
    if(other_queue_id < 0)
        error_exit("cannot access other client queue");

    enterChat(id, other_queue_id);
}

void getServerMessage() {
    Message* message = (Message*)malloc(sizeof(Message));

    if(msgrcv(client_queue_id, message, MSG_SIZE, 0, IPC_NOWAIT) >= 0) {
        if(message->m_type == 1) {
            printf("STOP from server, quitting...\n");
            handleSTOP();
        } else if(message->m_type == 5) {
            printf("Connecting to client %d...\n", message->client_id);
            int other_queue_id = msgget(message->queue_key, 0);
            if(other_queue_id < 1)
                error_exit("cannot access other client queue");
            enterChat(message->client_id, other_queue_id);
        }
    }
}

void endWork(int signum){
    handleSTOP();
}

int main(){
    srand(time(NULL));
    queue_key = ftok(getenv("HOME"), (rand() % 255 + 1));
    
    printf("Queue key: %d\n", queue_key);
    client_queue_id = msgget(queue_key, IPC_CREAT | 0666);
    if(client_queue_id < 0)
        error_exit("cannot create queue");
    printf("Queue ID: %d\n", client_queue_id);

    key_t server_key = ftok(getenv("HOME"), 1);
    server_queue_id = msgget(server_key, 0);
    if(server_queue_id < 0)
        error_exit("cannot access server queue");
    printf("Server queue ID: %d\n", server_queue_id);

    connectToServer();
    printf("ID received: %d\n", client_id);

    signal(SIGINT, endWork);

    char* command = NULL;
    size_t len = 0;
    ssize_t message_type;
    while(1) {
        printf("Enter command: ");
        message_type = getline(&command, &len, stdin);
        command[message_type - 1] = '\0';

        getServerMessage();

        if(strcmp(command, "") == 0)
            continue;

        char* tok = strtok(command, " ");
        if(strcmp(tok, "LIST") == 0) {
            printf("LIST command\n");
            handleLIST();
        } else if(strcmp(tok, "CONNECT") == 0) {
            tok = strtok(NULL, " ");
            int id = atoi(tok);
            handleCONNECT(id);
        } else if(strcmp(tok, "STOP") == 0) {
            printf("STOP command\n");
            handleSTOP();
        } else printf("Unrecognized command: %s\n", command);
    }

    return 0;
}