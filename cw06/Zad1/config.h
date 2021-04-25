#ifndef CONFIG_H
#define CONFIG_H

typedef enum m_type {
    STOP = 1, DISCONNECT = 2, INIT = 3, LIST = 4, CONNECT = 5
} m_type;

typedef struct Message {
    long m_type;
    char m_text[1024];
    key_t queue_key;
    int client_id;
    int receiver_id;
} Message;

const int MSG_SIZE = sizeof(Message) - sizeof(long);

#endif //CONFIG_H