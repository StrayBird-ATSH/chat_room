#include "wrapper.h"

#ifndef _SERVER_
#define _SERVER_

#define LOG_IN      '1'
#define QUIT      '2'
#define PRIVATE   '3'
#define BROADCAST    '4'
#define QUIT_CHAT     '7'
#define SHUT_SERVER      '8'
#define ONLINE         1
#define DOWNLINE       0
struct account {
    char name[20];
    char password[20];
};

struct list_node {
    struct account user_msg;
    int confd;
    struct list_node *next;
};

struct choice {
    char mode[10];
    struct account user_msg;
};

struct chat {
    char mode[10];
    char f_name[20];
    char m_name[20];
    char msg[256];
};

extern struct list_node *head;

extern long int codes[LISTENQ];

int traverse(struct list_node *);

int main();

int get_choice(int fd);

void *manage_client(void *arg);

void insert_node(struct list_node *load_user);

int log_in(int fd, struct account *temp_user);

struct list_node *find_online(char *);

int check_online(char *name);

struct list_node *create_node(struct account *user_msg, int conn_fd);

int manage_chat(int);

int remove_node(struct list_node *user_node);

int my_private(int, struct chat *);

int broadcast(struct chat *tran_msg);

int notice(char *, int);

int shut_server();

#endif
