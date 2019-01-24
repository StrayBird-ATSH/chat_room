#include "server.h"

/** The list of the structures of the currently online users */
struct list_node *head = 0;

/** A list of dynamically generated pass codes */
long int codes[LISTENQ];

/** A lock for making log in operation thread safe */
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
    int socket_fd = Open_listenfd("8000");
    pthread_t thread;
    codes[0] = 2019;
    while (1) {
        Fputs("Prepared to get connected...\n", stderr);
        int connection_fd = Accept(socket_fd, NULL, NULL);
        Pthread_create(&thread, 0, manage_client, (void *) (long) connection_fd);
    }
}

/** This function is an argument of the Pthread_create method and
 * will be executed when a new thread is created.
 * When the client quits, it disconnects with the server and the
 * corresponding server thread exits.*/
void *manage_client(void *arg) {
    int fd = (int) (long) arg;
    get_choice(fd);
    Pthread_exit((void *) 0);
    return NULL;
}

/** The main routine of the option-choosing interface, the loop will
 * not be broken until the client wants to quit.*/
int get_choice(int fd) {
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    struct choice tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    while (1) {
        Rio_readn(fd, &tran_msg, sizeof(tran_msg));
        switch (tran_msg.mode[0]) {
            case LOG_IN:
                log_in(fd, &tran_msg.user_msg);
                break;
            case QUIT:
                strcpy(buf, "You have exited successfully!\n");
                Rio_writen(fd, buf, MAXLINE);
                Close(fd);
                return 0;
            default:
                break;
        }
        memset(&tran_msg, 0, sizeof(tran_msg));
        memset(buf, 0, MAXLINE);
    }
}

/** The event handler for the log in option that handles
 * 1) Comparing the name of the current logged users with 
 *    the user that wants to log in to make sure that there
 *    won't be two users with the same name online at the same
 *    time;
 * 2) Compare the password with the current password list to 
 *    make sure that the user enters a valid password.
 * 3) Dynamically generate a new password for the newly logged
 *    user so that s/he can use this password to invite other 
 *    users into this chat room.
 * After handling the events above, it will transfer the control
 * to the manage chat function that handles massage sending and
 * receiving actions.
 * */
int log_in(int fd, struct account *temp_user) {
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    if (check_online(temp_user->name) == 1) {
        strcpy(buf, "The user have loaded, can't reload\n");
        Rio_writen(fd, buf, MAXLINE);
        return 0;
    }
    short pass = 0;
    char *end_ptr;
    long int code = strtol(temp_user->password, &end_ptr, 10);
    for (int i = 0; codes[i] != 0 && i < LISTENQ; ++i) {
        if (codes[i] == code) {
            pass = 1;
            break;
        }
    }
    if (pass == 0) {
        strcpy(buf, "Sorry, the pass code does not match\n");
        Rio_writen(fd, buf, MAXLINE);
        return 0;
    }
    while (1) {
        code = random();
        if (code == 0) continue;
        break;
    }
    for (int i = 0; i < LISTENQ; ++i) {
        if (codes[i] != 0) continue;
        codes[i] = code;
        break;
    }
    sprintf(buf, "Log in success: Welcome to chat room\n"
                 "The code generated for you is %ld\n", code);
    Rio_writen(fd, buf, MAXLINE);
    pthread_mutex_lock(&log_lock);
    struct list_node *temp = create_node(temp_user, fd);
    notice(temp_user->name, ONLINE);
    insert_node(temp);
    traverse(head);
    pthread_mutex_unlock(&log_lock);
    memset(buf, 0, MAXLINE);
    manage_chat(fd);
    return 0;
}

/** Send a message to all the online users telling them 
 * someone is online or offline.*/
int notice(char *name, int choice) {
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(struct chat));
    char temp_msg[256];
    memset(temp_msg, 0, 256);
    strncpy(temp_msg, name, (strlen(name) - 1));
    if (choice == ONLINE) strcat(temp_msg, " online!\n");
    if (choice == DOWNLINE) strcat(temp_msg, " has down line!\n");
    memset(&tran_msg, 0, sizeof(struct chat));
    strcpy(tran_msg.m_name, "Server\n");
    strcpy(tran_msg.msg, temp_msg);
    struct list_node *temp;
    for (temp = head; temp != NULL; temp = temp->next)
        Rio_writen(temp->confd, &tran_msg, sizeof(struct chat));
    return 0;
}

/** When a new user logs, the node of this user is created. */
struct list_node *create_node(struct account *user_msg, int conn_fd) {
    struct list_node *temp = (struct list_node *) malloc(sizeof(struct list_node));
    strcpy(temp->user_msg.name, user_msg->name);
    strcpy(temp->user_msg.password, user_msg->password);
    temp->confd = conn_fd;
    temp->next = NULL;
    return temp;
}

/** When a new user logs, the node of this user is inserted into the list. */
void insert_node(struct list_node *load_user) {
    if (head == NULL) {
        head = load_user;
        return;
    }
    load_user->next = head;
    head = load_user;
}

/** Displays the info of currently online users on the server side. */
int traverse(struct list_node *head) {
    struct list_node *stu;
    Fputs("Information of currently online users\n", stdout);
    for (stu = head; stu != NULL; stu = stu->next)
        fprintf(stderr, "name: %s password: %s", stu->user_msg.name, stu->user_msg.password);
    Fputs("Finished displaying currently online users\n", stdout);
    return 0;
}

/** Handles the specified requests. */
int manage_chat(int connection_fd) {
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    while (1) {
        Rio_readn(connection_fd, &tran_msg, sizeof(tran_msg));
        switch (tran_msg.mode[0]) {
            case PRIVATE:
                my_private(connection_fd, &tran_msg);
                break;
            case BROADCAST:
                broadcast(&tran_msg);
                break;
            case QUIT_CHAT: {
                struct list_node *temp_link = find_online(tran_msg.m_name);
                pthread_mutex_lock(&log_lock);
                remove_node(temp_link);
                notice(temp_link->user_msg.name, DOWNLINE);
                pthread_mutex_unlock(&log_lock);
                traverse(head);
                strcpy(tran_msg.m_name, "Server\n");
                strcpy(tran_msg.msg, "You have left the chat room\n");
                Rio_writen(connection_fd, &tran_msg, sizeof(tran_msg));
                return 0;
            }
            case SHUT_SERVER: {
                shut_server();
                Sleep(10);
                strcpy(tran_msg.m_name, "Server\n");
                strcpy(tran_msg.msg, "Server have been closed, You are forced off line!\n");
                struct list_node *temp;
                for (temp = head; temp != NULL; temp = temp->next)
                    Rio_writen(temp->confd, &tran_msg, sizeof(tran_msg));
                exit(0);
            }
            default:
                strcpy(tran_msg.msg, "The option does not exist\n");
                Rio_writen(connection_fd, &tran_msg, sizeof(tran_msg));
                break;
        }
        memset(&tran_msg, 0, sizeof(tran_msg));
    }
}

/** Checks if the user with the specified name is online. */
int check_online(char *name) {
    if (head == NULL) return 0;
    struct list_node *temp;
    for (temp = head; temp != NULL; temp = temp->next)
        if (strcmp(name, temp->user_msg.name) == 0)
            return 1;
    return 0;
}

/** Find the user with the specified name if s/he is online. */
struct list_node *find_online(char *user_name) {
    struct list_node *temp_user;
    for (temp_user = head; temp_user != NULL; temp_user = temp_user->next)
        if (strcmp(temp_user->user_msg.name, user_name) == 0)
            return temp_user;
    return NULL;
}

/**
 * Removes the node with the user information from the list
 * when the user is offline. */
int remove_node(struct list_node *user_node) {
    struct list_node *temp = NULL;
    if (user_node == NULL)
        return 1;
    if (user_node == head) {
        head = head->next;
        return 1;
    }
    for (temp = head; temp != NULL; temp = temp->next)
        if (temp->next == user_node) {
            temp->next = user_node->next;
            return 0;
        }
    return 1;
}

/** Handles the request of shutting down server. */
int shut_server() {
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    struct list_node *temp = NULL;
    strcpy(tran_msg.m_name, "Server\n");
    strcpy(tran_msg.msg,
           "Server will be close after 10 seconds, please go down line as soon as possible!\n");
    for (temp = head; temp != NULL; temp = temp->next)
        Rio_writen(temp->confd, &tran_msg, sizeof(tran_msg));
    return 0;
}

/** Handles the request of private chat. */
int my_private(int conn_fd, struct chat *temp) {
    struct list_node *temp_user = find_online(temp->f_name);
    if (temp_user == NULL) {
        strcpy(temp->m_name, "Server\n");
        strcpy(temp->msg, "The people you want to connect are not online\n");
        Rio_writen(conn_fd, temp, sizeof(struct chat));
        return 0;
    }
    int temp_conn_fd = temp_user->confd;
    Rio_writen(temp_conn_fd, temp, sizeof(struct chat));
    return 0;
}

/** Handles the request of broadcast. */
int broadcast(struct chat *tran_msg) {
    struct list_node *temp;
    for (temp = head; temp != NULL; temp = temp->next)
        Rio_writen(temp->confd, tran_msg, sizeof(struct chat));
    return 0;
}
