#ifndef CONFIG_H
#define CONFIG_H

#define NAME_LEN 30

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} m_type;

const int MAX_MSG_LEN = 32768;

#endif //CONFIG_H