/*
 * chatserver.c - A chat server
 * Communication protocol:
 *    JOIN name
 *    WHO
 *    LEAVE
 */

#include "nethelp.h"
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define MAX_CLIENTS 20

typedef struct
{
   int fd;
   char * name;  
} client_info;

client_info clients[MAX_CLIENTS];  /* slot i is empty is clients[i].name = NULL */
sem_t mutex;  /* semaphore to protect critical sections */

/* Thread function */
void * HandleClient(void * arg);

/* Add client name in position index in the array 'clients' */
void HandleJOIN(char * name, int index);

/* Write the names of all active clients over the connection fd */
void HandleWHO(int fd);

/* Remove the client from position index in the array 'clients' */
void HandleLEAVE(int index);

/* Broadcast message from client in position index to all active clients */
void HandleBroadcast(char * msg, int index);

int main(int argc, char **argv) 
{
    int listenfd, port;
    struct sockaddr clientaddr;
    int clientlen = sizeof(struct sockaddr);
    int connfd, i, tid;

    if (argc != 2) {
	   fprintf(stderr, "usage: %s <port>\n", argv[0]);
	   exit(0);
    }
    /* Get the port number on which to listen for incoming requests from clients */
    port = atoi(argv[1]);

    /* Initialize array of clients */
    for(i = 0; i < MAX_CLIENTS; i++)
    {
       clients[i].name = NULL;
       clients[i].fd = -1;
    }

    /* Initialize semaphore to protect critical sections */
    sem_init(&mutex, 0, 1);

    /* Create a listening socket */
    listenfd = open_listenfd(port);
    if(listenfd < 0)
    {
      printf("Failed to open listening socket\n");
      exit(1);
    }

    while (1) {
       /* Accept an incoming request from a client */ 
	     connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
       if(connfd < 0) continue;  /* Connection failed */ 

       /* Allocate new entry in the array clients for the new client */
       /* Create a new thread to handle the client */
       sem_wait(&mutex);
       for(i = 0; i < MAX_CLIENTS; i++)
       {
          if(clients[i].fd < 0) /* empty slot */
          {
            clients[i].fd = connfd;
            pthread_create(&tid, NULL, HandleClient, (void*)i);
            break;
          }
       }
       sem_post(&mutex);
    }
}

/*
 * HandleClient - handle chat client
 * arg is the client index in the clients array
 */
void * HandleClient(void * arg) 
{
    size_t n; 
    char buf[MAXLINE];

    int index = (int)arg;  
    int connfd = clients[index].fd; 

    /* Detach the thread to free memory resources upon termination */
    pthread_detach(pthread_self());

    /* Read a line from the client */
    n = readline(connfd, buf, MAXLINE); 

    while(n > 0) {
       /* Process commands JOIN, WHO, LEAVE */
       /* Broadcast anything else */
       if(strncmp(buf, "JOIN", 4) == 0)
        {
           HandleJOIN(buf+4, index);
         }
       else if(strncmp(buf, "WHO", 3) == 0)
            HandleWHO(connfd);
       else if(strncmp(buf, "LEAVE", 5) == 0)
            HandleLEAVE(index);
       else
            HandleBroadcast(buf, index);

       /* Read the next line from the client */
       n = readline(connfd, buf, MAXLINE);
    }
    /* Close connection with client */
    close(connfd);
    return NULL;
}

/*
 * Send out client names in response to the WHO command
 */
void HandleWHO(int fd)
{
   int i;

   sem_wait(&mutex);
   for(i = 0; i < MAX_CLIENTS; i++)
       if(clients[i].name != NULL) {
          write(fd, clients[i].name, strlen(clients[i].name));
          write(fd, "\n", 1);
        }
  sem_post(&mutex);
}

/*
 * Broadcast msg from client in position index to all active chat clients
 */
void HandleBroadcast(char * msg, int index)
{
    int i;
    char buf[MAXLINE];
    
    if(clients[index].name == NULL) return;  /* client doesn't exist */
    
    sprintf(buf, "%s: %s", clients[index].name, msg);
    
    /* 'clients' may be modified by other threads, protect with semaphores */
    sem_wait(&mutex);
    for(i = 0; i < MAX_CLIENTS; i++)
        if(clients[i].name != NULL)
            write(clients[i].fd, buf, strlen(buf));
    sem_post(&mutex);
}

/*
 * Add client name in position index in the array clients
 */
void HandleJOIN(char * name, int index)
{
    char buf[MAXLINE];
    int i;

    /* Skip all the leading whitespaces */
    while((*name == ' ') || (*name == '\t')) name++;

    /* Replace '\n' by '\0' in user name */
    if(name[strlen(name)-1] == '\n') 
       name[strlen(name)-1] = '\0';

    /* Make sure that client did not already join */
    if(clients[index].name != NULL) {
       sprintf(buf, "Already joined as %s\n", clients[index].name);
       write(clients[index].fd, buf, strlen(buf));
       return;
    }

    sem_wait(&mutex);
    clients[index].name = malloc(strlen(name)+1);
    strcpy(clients[index].name, name);

    for(i = 0; i < MAX_CLIENTS; i++)
	  {
       if(clients[i].name == NULL) continue;
       if(i == index) 
          sprintf(buf, "Welcome to the chat room, %s!\n", name);
       else
          sprintf(buf, "%s has joined the chat room \n", name);
       write(clients[i].fd, buf, strlen(buf));
	  }
    sem_post(&mutex);
 }

/*
 * Remove client from position index in array clients
 * and close the connection to the client 
 */
void HandleLEAVE(int index)
{
   sem_wait(&mutex);
   if(clients[index].name != NULL) 
   {
      free(clients[index].name);
      clients[index].name = NULL;
   }
   close(clients[index].fd);
   sem_post(&mutex);

   /* Terminate thread */
   pthread_exit(NULL);
}

