#ifndef CONFIG_H
#define CONFIG_H

typedef enum type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} type;

typedef struct Message {
    long type;
    key_t queue_key;
    int client_id;
    int receiver_id;
    char text[1024];
} Message;

const int MESSAGE_SIZE = sizeof(Message) - sizeof(long);

#endif