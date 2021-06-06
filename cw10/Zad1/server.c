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
}Client;

Client *clients[MAX_PLAYERS];
int NO_clients = 0;

int addNewUser(char *name, int client_desc){
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(clients[i] != NULL && strcmp(clients[i]->name, name) == 0){
            printf("XDDDDDD\n");
            send(client_desc, "connect:username_taken", MESSAGE_LEN, 0);
            close(client_desc);
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
        new_client->desc = client_desc;
        new_client->online = 1;

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
                send(clients[i]->desc, "ping: ", MESSAGE_LEN, 0);
                clients[i]->online = 0;
            }
        }

        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
}

void createLocalSocket(char *path){
    local_socket = socket(AF_UNIX, SOCK_STREAM, 0);

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
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &info);

    network_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    bind(network_socket, info->ai_addr, info->ai_addrlen);
    listen(network_socket, MAX_BACKLOG);

    freeaddrinfo(info);
}

int receiveMessages(){
    struct pollfd *descriptors = calloc(2 + NO_clients, sizeof(struct pollfd));
    descriptors[0].fd = local_socket;
    descriptors[0].events = POLLIN;
    descriptors[1].fd = network_socket;
    descriptors[1].events = POLLIN;

    pthread_mutex_lock(&mutex);
    for(int i = 0; i < NO_clients; i++){
        descriptors[i + 2].fd = clients[i]->desc;
        descriptors[i + 2].events = POLLIN;
    }
    pthread_mutex_unlock(&mutex);
    
    //infinite waiting
    poll(descriptors, NO_clients + 2, -1);

    int client_desc;
    for(int i = 0; i < NO_clients + 2; i++){
        if(descriptors[i].revents & POLLIN){
            client_desc = descriptors[i].fd;
            break;
        }
    }

    if(client_desc == network_socket || client_desc == local_socket)
        client_desc = accept(client_desc, NULL, NULL);
    
    free(descriptors);
    return client_desc;
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
        int client_desc = receiveMessages();
        char message[MESSAGE_LEN + 1];
        recv(client_desc, message, MESSAGE_LEN, 0);
        printf("%s\n", message);
        char *command = strtok(message, ":");
        char *selected_field = strtok(NULL, ":");
        char *username = strtok(NULL, ":");

        pthread_mutex_lock(&mutex);
        if (strcmp(command, "connect") == 0){
            int id = addNewUser(username, client_desc);
            if(id != -1){
                if(id % 2 == 0)
                    send(client_desc, "connect:no_opponent", MESSAGE_LEN, 0);
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

                    send(clients[player_1]->desc, "set_symbol:O", MESSAGE_LEN, 0);
                    send(clients[player_2]->desc, "set_symbol:X", MESSAGE_LEN, 0);
                }
            }
            
        } else if(strcmp(command, "move") == 0){
            int player = getPlayer(username);
            sprintf(message, "move:%d", atoi(selected_field));
            send(clients[getOpponent(player)]->desc, message, MESSAGE_LEN, 0);
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