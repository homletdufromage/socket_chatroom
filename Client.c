#include "Client.h"
#include <stdlib.h>

struct client_t {
    int socket;
    int id;
    char* ip;
};

Client* create_client(int sock, int id, char* ip) {
    Client* cli = malloc(sizeof(Client));
    if (!cli)
        return NULL;
    cli->socket = sock;
    cli->id = id;
    cli->ip = ip;

    return cli;
}

void destroy_client(Client* cli) {
    free(cli->ip);
    free(cli);
}

/**
 * GETTERS
 */
int getSocket(Client* cli) {
    return cli->socket;
}
int getId(Client* cli) {
    return cli->id;
}
char* getIp(Client* cli) {
    return cli->ip;
}

/**
 * CLIENTS LIST
 */
int add_client(Client* cli, Client** list, const unsigned LIST_SIZE) {
    for(int i = 0; i < LIST_SIZE; i++) {
        if (list[i] == NULL){
            list[i] = cli;
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}

int remove_client(int clientId, Client** list, const unsigned LIST_SIZE) {
    for(int i = 0; i < LIST_SIZE; i++) {
        if (list[i] && list[i]->id == clientId) {
            destroy_client(list[i]);
            list[i] = NULL;
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}