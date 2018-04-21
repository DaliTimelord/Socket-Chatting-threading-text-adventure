
#ifndef __NET_HELP
#define __NET_HELP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>      /* for fgets */
#include <strings.h>     /* for bzero, bcopy */
#include <unistd.h>      /* for read, write */
#include <sys/socket.h>  /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>     /* for thread use */

#define MAXLINE  8192  /* max text line length */
#define LISTENQ  1024  /* second argument to listen() */

/* 
 * open_listenfd - open and return a listening socket on port;
 * return -1 in case of failure 
 */
int open_listenfd(int port);

/*
 * open_clientfd - open connection to server at 
 * and return a socket descriptor ready for reading and writing;
 * return < 0 in case of failure.
 */
int open_clientfd(char *hostname, int port);

/*
 * readline - read a line of text ending with '\0'
 * return the number of characters read;
 * return -1 if error
 */
int readline(int fd, char * buf, int maxlen);

#endif