#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_PLAYERS 20
#define MAX_BACKLOG 10
#define MESSAGE_LEN 256

int local_socket;
int network_socket;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct{
    char *name;
    int desc;
    int online;
    struct sockaddr addr;
}Client;

Client *clients[MAX_PLAYERS];
int NO_clients = 0;

int addNewUser(char *name, int socket_desc, struct sockaddr addr){
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0){
            sendto(socket_desc, "connect:username_taken", MESSAGE_LEN, 0, (struct sockaddr *)&addr, sizeof(struct addrinfo));
            return -1;
        }
    }

    int id = -1;

    for(int i = 0; i < MAX_PLAYERS; i += 2){
        if(clients[i] != NULL && clients[i + 1] == NULL){
            id = i + 1;
            break;
        }
    }

    if(id == -1){
        for(int i = 0; i < MAX_PLAYERS; i++){
            if(clients[i] == NULL){
                id = i;
                break;
            }
        }
    }

    if(id != -1){
        Client *new_client = malloc(sizeof(Client));
        new_client->name = calloc(MESSAGE_LEN, sizeof(char));
        strcpy(new_client->name, name);
        new_client->desc = socket_desc;
        new_client->online = 1;
        new_client->addr = addr;

        clients[id] = new_client;
        NO_clients++;
    }

    return id;
}

int getOpponent(int id){
    if(id % 2 == 0)
        return id + 1;
    else
        return id - 1;
}

int getPlayer(char *name){
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
            return i;
    }
    return -1;
}

void freeUser(int index){
    free(clients[index]->name);
    free(clients[index]);
    clients[index] = NULL;
    NO_clients--;
    int opponent = getOpponent(index);

    if(clients[opponent] != NULL){
        printf("Removing opponent: %s\n", clients[opponent]->name);
        sendto(clients[opponent]->desc, "disconnect: ", MESSAGE_LEN, 0, &clients[opponent]->addr, sizeof(struct addrinfo));
        free(clients[opponent]->name);
        free(clients[opponent]);
        clients[opponent] = NULL;
        NO_clients--;
    }
}

void disconnectUser(char *name){
    int index = -1;
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
            index = i;
    }
    printf("Removing client: %s\n", name);
    freeUser(index);
}

void ping(){
    while(1){
        printf("Pinging...\n");

        pthread_mutex_lock(&mutex);

        //disconnect inactive users
        for(int i = 0; i < MAX_PLAYERS; i++){
            if(clients[i] != NULL && !clients[i]->online)
                disconnectUser(clients[i]->name);
        }

        //send ping to clients
        for(int i = 0; i < MAX_PLAYERS; i++){
            if(clients[i] != NULL){
                sendto(clients[i]->desc, "ping: ", MESSAGE_LEN, 0, &clients[i]->addr, sizeof(struct addrinfo));
                clients[i]->online = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
}

void createLocalSocket(char *path){
    local_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct sockaddr_un local_sockaddr;
    memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
    local_sockaddr.sun_family = AF_UNIX;
    strcpy(local_sockaddr.sun_path, path);

    unlink(path);
    bind(local_socket, (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr_un));
    listen(local_socket, MAX_BACKLOG);
}

void createNetworkSocket(char *port){
    struct addrinfo *info;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &info);

    network_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    bind(network_socket, info->ai_addr, info->ai_addrlen);
    listen(network_socket, MAX_BACKLOG);

    freeaddrinfo(info);
}

int receiveMessages(){
    struct pollfd fds[2];
    fds[0].fd = local_socket;
    fds[0].events = POLLIN;
    fds[1].fd = network_socket;
    fds[1].events = POLLIN;
    poll(fds, 2, -1);
    for (int i = 0; i < 2; i++){
        if (fds[i].revents & POLLIN)
            return fds[i].fd;
    }
    return -1;
}

int main(int argc, char *argv[]){
    if (argc != 3)
        exit(1);

    srand(time(NULL));
    createNetworkSocket(argv[1]);
    createLocalSocket(argv[2]);

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, (void *(*)(void *))ping, NULL);

    while(1){
        int socket_desc = receiveMessages();
        char message[MESSAGE_LEN + 1];
        struct sockaddr from_addr;
        socklen_t from_length = sizeof(struct sockaddr);
        recvfrom(socket_desc, message, MESSAGE_LEN, 0, &from_addr, &from_length);
        
        printf("%s\n", message);
        char *command = strtok(message, ":");
        char *selected_field = strtok(NULL, ":");
        char *username = strtok(NULL, ":");

        pthread_mutex_lock(&mutex);
        if (strcmp(command, "connect") == 0){
            int id = addNewUser(username, socket_desc, from_addr);
            
            if(id != -1){
                if(id % 2 == 0)
                    sendto(socket_desc, "connect:no_opponent", MESSAGE_LEN, 0, (struct sockaddr *)&from_addr, sizeof(struct addrinfo));
                else{
                    int player_1, player_2;
                    if((rand() % 2 + 1) == 1){
                        player_1 = id;
                        player_2 = getOpponent(id);
                    }
                    else{
                        player_2 = id;
                        player_1 = getOpponent(id);
                    }

                    sendto(clients[player_1]->desc, "set_symbol:O", MESSAGE_LEN, 0, &clients[player_1]->addr, sizeof(struct addrinfo));
                    sendto(clients[player_2]->desc, "set_symbol:X", MESSAGE_LEN, 0, &clients[player_2]->addr, sizeof(struct addrinfo));
                }
            }
        } else if(strcmp(command, "move") == 0){
            int player = getPlayer(username);
            sprintf(message, "move:%d", atoi(selected_field));
            sendto(clients[getOpponent(player)]->desc, message, MESSAGE_LEN, 0, &clients[getOpponent(player)]->addr, sizeof(struct addrinfo));
        } else if(strcmp(command, "disconnect") == 0){
            disconnectUser(username);
        } else if(strcmp(command, "pong") == 0){
            int player = getPlayer(username);
            if(player != -1)
                clients[player]->online = 1;
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}