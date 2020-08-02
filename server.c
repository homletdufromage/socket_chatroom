#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "Client.h"

#define PORT "11037"
#define BACKLOG 3
#define BUFFER_SIZE 1024
#define MAX_CLIENTS_NB 10

int clientsNb = 0;
Client* clients[MAX_CLIENTS_NB];

void sendAll(char* msg, int idToIgnore) {
    for(int i = 0; i < MAX_CLIENTS_NB; i++) {
        if (clients[i] && getId(clients[i]) != idToIgnore)
            send(getSocket(clients[i]), msg, strlen(msg), 0);
    }
}

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void* handle_client(void* arg) {
    Client* cli = (Client*) arg;

    int msgSize = 0;
    char buffer[BUFFER_SIZE];
    buffer[0] = '\0';
    char mess[BUFFER_SIZE];

    sprintf(mess, "%s[%d] joined the chat.\n", getIp(cli), getId(cli));
    sendAll(mess, getId(cli));

    while ((msgSize = recv(getSocket(cli), buffer, BUFFER_SIZE, 0)) != 0) {
        if (msgSize != -1) {
            buffer[msgSize] = '\0';
            bzero(mess, BUFFER_SIZE);
            if (strncmp(buffer, "close", 5) == 0)
                break;
            printf("%d bytes received from %s[%d] : %s", msgSize, getIp(cli), getId(cli), buffer);
            
            snprintf(mess, BUFFER_SIZE, "[%d]%s : %s", getId(cli), getIp(cli), buffer);
            sendAll(mess, getId(cli));
        }
    }

    printf("%s[%d] left the chat.\n", getIp(cli), getId(cli));
    sprintf(mess, "%s[%d] left the chat.\n", getIp(cli), getId(cli));
    
    sendAll(mess, getId(cli));
    close(getSocket(cli));

    if (remove_client(getId(cli), clients, MAX_CLIENTS_NB) != EXIT_SUCCESS)
        printf("Client couldn't be removed from the list\n");

    pthread_detach(pthread_self());
    
    return NULL;
}

int create_socket() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (!p)
        return -1;

    return sockfd;
}

void run_server(int sockfd) {
    int new_fd;
    struct sockaddr_storage their_addr;

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    while(1) {
        socklen_t sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        clientsNb++;

        // Gets the client's IP address in string format
        char* ip = malloc(INET_ADDRSTRLEN * sizeof(char));
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr*)&their_addr),
                  ip,  INET_ADDRSTRLEN*sizeof(char));

        // Creating a new client
        Client* newClient = create_client(new_fd, clientsNb, ip);
        int status = add_client(newClient, clients, MAX_CLIENTS_NB);
        if (status != EXIT_SUCCESS){
            char* errmess = "Sorry, we're full!\n";
            send(new_fd, errmess, strlen(errmess), 0);
            printf("Client %s couldn't be added to the list\n", ip);
            destroy_client(newClient);
            close(new_fd);
        }
        else {
            printf("New connection from %s\n", ip);

            pthread_t threadId;
            if (pthread_create(&threadId, NULL, handle_client, newClient)) {
                perror("pthread_create");
                break;
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    int sockfd;

    printf("Creating the server's socket...");
    if ((sockfd = create_socket()) == -1) {
        fprintf(stderr, "unable to create the server's socket\n");
        exit(1);
    }
    printf("OK !\n");

    bzero(clients, MAX_CLIENTS_NB);

    printf("Server: waiting for connections...\n");
    run_server(sockfd);

    return 0;
}