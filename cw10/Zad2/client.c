#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MESSAGE_LEN 256

int server_socket;
int binded_socket;
int is_client_O;
char message[MESSAGE_LEN + 1];
char *username;

char *command, *arg;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct{
    int move;
    int symbols[9];
} Game;

int move(Game *game, int field){
    if (field < 0 || field > 9 || game->symbols[field] != 0)
        return 0;
    game->symbols[field] = game->move ? 1 : 2;
    game->move = !game->move;
    return 1;
}

int checkWinner(Game *game){
    //check rows
    int symbol;
    for(int i = 0; i < 3; i++){
        symbol = game->symbols[3 * i];
        if(symbol != 0 && symbol == game->symbols[3 * i + 1] && symbol == game->symbols[3 * i + 2])
            return symbol;
    }

    //check columns
    for (int i = 0; i < 3; i++){
        symbol = game->symbols[i];
        if (symbol != 0 && symbol == game->symbols[i + 3] && symbol == game->symbols[i + 6])
            return symbol;
    }

    //check first diagonal
    symbol = game->symbols[0];
    if(symbol != 0 && symbol == game->symbols[4] && symbol == game->symbols[8])
        return symbol;

    //check second diagonal
    symbol = game->symbols[2];
    if(symbol != 0 && symbol == game->symbols[4] && symbol == game->symbols[6])
        return symbol;
    else return 0;
}Game game;

typedef enum{
    START,
    WAITING_FOR_OPPONENT,
    WAITING_FOR_MOVE,
    MOVE_OPPONENT,
    MOVING,
    DISCONNECT
} ClientStatus;

ClientStatus status = START;

void disconnect()
{
    char message[MESSAGE_LEN + 1];
    sprintf(message, "disconnect: :%s", username);
    send(server_socket, message, MESSAGE_LEN, 0);
    exit(0);
}

void checkForGameResult(){
    int win = 0;
    int winner = checkWinner(&game);
    if(winner != 0){
        if ((is_client_O && winner == 1) || (!is_client_O && winner == 2))
            printf("You have WON!\n");
        else printf("You have LOST!\n");

        win = 1;
    }

    int draw = 1;
    for(int i = 0; i < 9; i++){
        if(game.symbols[i] == 0){
            draw = 0;
            break;
        }
    }

    if(draw && !win)
        printf("That's a DRAW\n");

    if(win || draw)
        status = DISCONNECT;
}

Game newBoard(){
    Game game = {1, {0}};
    return game;
}

void drawBoard(){
    char symbol;
    for(int y = 0; y < 3; y++){
        for(int x = 0; x < 3; x++){
            if (game.symbols[y * 3 + x] == 0)
                symbol = y * 3 + x + 1 + '0';
            
            else if (game.symbols[y * 3 + x] == 1)
                symbol = 'O';
            
            else symbol = 'X';
            printf("  %c  ", symbol);
        }
        printf("\n----------------\n");
    }
}

void startGame(){
    while(1){
        if(status == START){
            if(strcmp(arg, "no_opponent") == 0){
                printf("Waiting for opponent\n");
                status = WAITING_FOR_OPPONENT;
            }
            else{
                game = newBoard();
                is_client_O = arg[0] == 'O';
                status = is_client_O ? MOVING : WAITING_FOR_MOVE;
            }
        }
        else if(status == WAITING_FOR_OPPONENT){
            pthread_mutex_lock(&mutex);
            while (status != START && status != DISCONNECT)
                pthread_cond_wait(&cond, &mutex);
            
            pthread_mutex_unlock(&mutex);

            game = newBoard();
            is_client_O = arg[0] == 'O';
            status = is_client_O ? MOVING : WAITING_FOR_MOVE;
        } else if(status == WAITING_FOR_MOVE){
            printf("Waiting for opponents move\n");

            pthread_mutex_lock(&mutex);
            while (status != MOVE_OPPONENT && status != DISCONNECT)
                pthread_cond_wait(&cond, &mutex);

            pthread_mutex_unlock(&mutex);
        } else if(status == MOVE_OPPONENT){
            move(&game, atoi(arg));
            checkForGameResult();
            if(status != DISCONNECT)
                status = MOVING;
        } else if(status == MOVING){
            drawBoard();
            int pos;
            do{
                printf("Next move (%c): ", is_client_O ? 'O' : 'X');
                scanf("%d", &pos);
                pos--;
            } while(!move(&game, pos));

            drawBoard();

            char message[MESSAGE_LEN + 1];
            sprintf(message, "move:%d:%s", pos, username);
            send(server_socket, message, MESSAGE_LEN, 0);

            checkForGameResult();

            if(status != DISCONNECT)
                status = WAITING_FOR_MOVE;
                
        } else if(status == DISCONNECT)
            disconnect();
    }
}

void connectToServer(char *connectionType, char *address){
    char message[MESSAGE_LEN + 1];
    sprintf(message, "connect: :%s", username);

    struct sockaddr_un local_sockaddr;
    if (strcmp(connectionType, "local") == 0){
        server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

        memset(&local_sockaddr, 0, sizeof(struct sockaddr_un));
        local_sockaddr.sun_family = AF_UNIX;
        strcpy(local_sockaddr.sun_path, address);

        connect(server_socket, (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr_un));
        binded_socket = socket(AF_UNIX, SOCK_DGRAM, 0);

        struct sockaddr_un binded_sockaddr;
        memset(&binded_sockaddr, 0, sizeof(struct sockaddr_un));
        binded_sockaddr.sun_family = AF_UNIX;
        
        sprintf(binded_sockaddr.sun_path, "%d", getpid());
        bind(binded_socket, (struct sockaddr *)&binded_sockaddr, sizeof(struct sockaddr_un));

        sendto(binded_socket, message, MESSAGE_LEN, 0, (struct sockaddr *)&local_sockaddr, sizeof(struct sockaddr_un));
    } else{
        struct addrinfo *info;

        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        getaddrinfo("127.0.0.1", address, &hints, &info);
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        connect(server_socket, info->ai_addr, info->ai_addrlen);

        freeaddrinfo(info);

        send(server_socket, message, MESSAGE_LEN, 0);
    }
}

void checkServerMessages(char *connectionType){
    int gameStarted = 0;
    while (1){
        if(strcmp(connectionType, "local") == 0){
            recv(binded_socket, message, MESSAGE_LEN, 0);
        }
        else{
            recv(server_socket, message, MESSAGE_LEN, 0);
        }

        command = strtok(message, ":");
        arg = strtok(NULL, ":");

        pthread_mutex_lock(&mutex);
        if(strcmp(arg, "username_taken") == 0){
                printf("Username is taken!\n");
                exit(1);
        } else if(strcmp(command, "set_symbol") == 0){
            status = START;
            if (!gameStarted){
                pthread_t game_thread;
                pthread_create(&game_thread, NULL, (void *(*)(void *))startGame, NULL);
                gameStarted = 1;
            }
        }
        else if(strcmp(command, "move") == 0){
            status = MOVE_OPPONENT;
        }
        else if(strcmp(command, "disconnect") == 0){
            status = DISCONNECT;
            exit(0);
        }
        else if(strcmp(command, "ping") == 0){
            sprintf(message, "pong: :%s", username);
            send(server_socket, message, MESSAGE_LEN, 0);
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 4)
        exit(1);

    username = argv[1];

    signal(SIGINT, disconnect);
    connectToServer(argv[2], argv[3]);

    checkServerMessages(argv[2]);
    return 0;
}