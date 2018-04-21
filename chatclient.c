/*
 * chatclient.c - A chat client
 * Communication protocol:
 *    JOIN name
 *    WHO
 *    LEAVE
 */

#include "nethelp.h"
#include <pthread.h>
#include <string.h>

void * HandleFeedback(void * fd);


int main(int argc, char *argv[]) 
{
    int clientfd, port, tid;
    char *host, buf[MAXLINE];

    if (argc != 3) {
     fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
     exit(0);
    }
    /* Get name of machine on which server is running */
    host = argv[1];          

    /* Get port number on which server is listening for client requests */
    port = atoi(argv[2]);

    /* Open a connection to the server */
    clientfd = open_clientfd(host, port);
    if(clientfd < 0) {
       printf("Connection to <%s, %d> failed\n", host, port);
       exit(0);
    }

    /* Create a thread to read messages from server and print them on the screen */
    pthread_create(&tid, NULL, HandleFeedback, (void*)clientfd);

    /* In a loop, read strings from user and send them to the server */
    /* Terminate when the user types in LEAVE */
    while(1) /* infinite loop */
    {
      fgets(buf, MAXLINE, stdin);
      write(clientfd, buf, strlen(buf));
      if(strncmp(buf, "LEAVE", 5) == 0)
          break;
    }

    close(clientfd);
    return 0;
}

void * HandleFeedback(void * fd)
{
    int connfd = (int)fd;
    char buf[MAXLINE];
    int n;

    pthread_detach(pthread_self());

    n = read(connfd, buf, MAXLINE);
    while (n > 0)
    {
      write(1, buf, n);
      n = read(connfd, buf, MAXLINE);
    }

    /* connection terminated, exit */
    pthread_exit(NULL);
}