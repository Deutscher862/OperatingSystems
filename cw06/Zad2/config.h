#ifndef CONFIG_H
#define CONFIG_H

#define NAME_LEN 5

typedef enum type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} type;

const int MAX_MESSAGE_SIZE = 256;

#endif