#include "client.h"

int sock_fd = 0;
char my_name[20];

/**
 * The main routine of the client side that starts listening a port,
 * displays the welcome and select option information and sends and
 * receives the package of option to select and the response from the
 * server.
 * This routine of this loop will not exit until the user selects the
 * quit option.
 * */
int main() {
    sock_fd = Open_clientfd("127.0.0.1", "8000");
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    struct choice tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    while (1) {
        printf("\nWelcome to the chat room.\n1 for login, 2 for exit\n");
        Fputs("Please give your choice:", stderr);
        Fgets(tran_msg.mode, sizeof(tran_msg.mode), stdin);
        if (strlen(tran_msg.mode) != 2) {
            Fputs("error: Please give your choice again\n", stderr);
            continue;
        }
        switch (tran_msg.mode[0]) {
            case LOG_IN:
                fprintf(stdout, "Please input your name:");
                Fgets(tran_msg.user_msg.name, sizeof(tran_msg.user_msg.name), stdin);
                fprintf(stdout, "Please input your password:");
                Fgets(tran_msg.user_msg.password, sizeof(tran_msg.user_msg.password), stdin);
                strcpy(my_name, tran_msg.user_msg.name);
                Rio_writen(sock_fd, &tran_msg, sizeof(tran_msg));
                Rio_readn(sock_fd, buf, MAXLINE);
                Fputs(buf, stderr);
                if (strcmp("The user have loaded, can't reload\n", buf) == 0)
                    break;
                if (strcmp("Sorry, the pass code does not match\n", buf) == 0)
                    break;
                manage_chat();
                break;
            case QUIT:
                Rio_writen(sock_fd, &tran_msg, sizeof(tran_msg));
                Rio_readn(sock_fd, buf, MAXLINE);
                Fputs(buf, stderr);
                Close(sock_fd);
                return 0;
            default:
                Fputs("Your choice is error, please give your choice again\n", stderr);
                break;
        }
        memset(buf, 0, MAXLINE);
        memset(&tran_msg, 0, sizeof(tran_msg));
    }
}

/** The control is transferred to this method if the user successfully logs in.*/
int manage_chat() {
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    pthread_t thread;
    Pthread_create(&thread, 0, read_serv, (void *) 0);
    while (1) {
        printf("\n3 for private chat\n4 for broadcast chat\n"
               "7 for exit\n8 for shut down the server\n");
        Fgets(tran_msg.mode, sizeof(tran_msg.mode), stdin);
        if (strlen(tran_msg.mode) != 2) {
            Fputs("Your choice does not conform to rules\n", stderr);
            continue;
        }
        switch (tran_msg.mode[0]) {
            case PRIVATE:
                my_private(&tran_msg);
                break;
            case BROADCAST:
                broadcast(&tran_msg);
                break;
            case QUIT_CHAT:
                strcpy(tran_msg.m_name, my_name);
                Rio_writen(sock_fd, &tran_msg, sizeof(tran_msg));
                Pthread_join(thread, NULL);
                return 0;
            case SHUT_SERVER:
                if (strcmp("admin\n", my_name) != 0) {
                    Fputs("You are not administrator! Can't use this function!\n", stderr);
                    break;
                }
                strcpy(tran_msg.m_name, my_name);
                Rio_writen(sock_fd, &tran_msg, sizeof(tran_msg));
                break;
            default:
                Fputs("The option does not exist, please input again.\n", stderr);
                break;
        }
        memset(&tran_msg, 0, sizeof(tran_msg));
    }
}

/**
 * When the user logs in, a new thread is created and the new thread will
 * enter this method that keeps receiving packages from the server side
 * displaying information of the message from others and the server.
 * */
void *read_serv(void *arg) {
    struct chat tran_msg;
    memset(&tran_msg, 0, sizeof(tran_msg));
    while (1) {
        ssize_t n_read = Rio_readn(sock_fd, &tran_msg, sizeof(tran_msg));
        if (n_read <= 0) {
            Fputs("read_serv:read failure\n", stderr);
            Close(sock_fd);
            Pthread_exit((void *) -1);
        }
        Fputs("Receive msg from:", stderr);
        Fputs(tran_msg.m_name, stderr);
        Fputs("Msg:", stderr);
        Fputs(tran_msg.msg, stderr);
        if (strcmp("You have left the chat room\n", tran_msg.msg) == 0)
            Pthread_exit((void *) 0);
        if (strcmp("Server have been closed, You are forced off line!\n", tran_msg.msg) == 0)
            exit(0);
        memset(&tran_msg, 0, sizeof(tran_msg));
    }
}

/** The handler method of private chat. */
int my_private(struct chat *temp) {
    strcpy(temp->m_name, my_name);
    Fputs("Friend name:", stderr);
    Fgets(temp->f_name, sizeof(temp->f_name), stdin);
    Fputs("I said:", stderr);
    Fgets(temp->msg, sizeof(temp->msg), stdin);
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    char time_tag[18];
    sprintf(time_tag, "\n(Sent at %02d:%02d)\n", time_info->tm_hour, time_info->tm_min);
    strcat(temp->msg, time_tag);
    Rio_writen(sock_fd, temp, sizeof(struct chat));
    return 0;
}

/** The handler method of broadcast. */
int broadcast(struct chat *tran_msg) {
    strcpy(tran_msg->m_name, my_name);
    Fputs("I said:", stderr);
    Fgets(tran_msg->msg, sizeof(tran_msg->msg), stdin);
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    char time_tag[18];
    sprintf(time_tag, "\n(Sent at %02d:%02d)\n", time_info->tm_hour, time_info->tm_min);
    strcat(tran_msg->msg, time_tag);
    Rio_writen(sock_fd, tran_msg, sizeof(struct chat));
    return 0;
}