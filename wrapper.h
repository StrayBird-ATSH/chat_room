#ifndef _WRAPPER_
#define _WRAPPER_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>

/* Misc constants */
#define    MAXLINE     8192  /* Max text line length */
#define LISTENQ  1024  /* Second argument to listen() */

/* Our own error-handling functions */
void unix_error(char *msg);

void posix_error(int code, char *msg);

void gai_error(int code, char *msg);

void app_error(char *msg);

/* Process control wrappers */
unsigned int Sleep(unsigned int secs);


/* Unix I/O wrappers */
void Close(int fd);


/* Standard I/O wrappers */
char *Fgets(char *ptr, int n, FILE *stream);


void Fputs(const char *ptr, FILE *stream);


/* Sockets interface wrappers */
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);


int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);


/* Protocol independent wrappers */
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res);

void Freeaddrinfo(struct addrinfo *res);


/* Pthreads thread control wrappers */
void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp,
                    void *(*routine)(void *), void *argp);

void Pthread_join(pthread_t tid, void **thread_return);


void Pthread_exit(void *retval);


/* Rio (Robust I/O) package */
ssize_t rio_readn(int fd, void *usrbuf, size_t n);

ssize_t rio_writen(int fd, void *usrbuf, size_t n);


/* Wrappers for Rio package */
ssize_t Rio_readn(int fd, void *usrbuf, size_t n);

void Rio_writen(int fd, void *usrbuf, size_t n);

/* Reentrant protocol-independent client/server helpers */
int open_clientfd(char *hostname, char *port);

int open_listenfd(char *port);

/* Wrappers for reentrant protocol-independent client/server helpers */
int Open_clientfd(char *hostname, char *port);

int Open_listenfd(char *port);


#endif /* _WRAPPER_ */
