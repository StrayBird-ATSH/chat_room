#include "wrapper.h"

#define LOG_IN     '1'
#define QUIT     '2'
#define PRIVATE   '3'
#define BROADCAST    '4'
#define QUIT_CHAT     '7'
#define SHUT_SERVER      '8'

struct account    /** Defines the structure used to store the account info of a user*/
{
    char name[20];
    char password[20];
};

struct choice {
    char mode[10];
    struct account user_msg;
};

/** Defines the structure that is used to store the information of a chat */
struct chat {
    char mode[10];
    char f_name[20];
    char m_name[20];
    char msg[256];
};

int manage_chat();

void *read_serv(void *);

int my_private(struct chat *);

int broadcast(struct chat *tran_msg);
