#ifndef __CLIENT__
#define __CLIENT__

typedef struct client_t Client;

/**
 * Accessors
 */
Client* create_client(int sock, int id, char* ip);
void destroy_client(Client* cli);

/**
 * GETTERS
 */
int getSocket(Client* cli);
int getId(Client* cli);
char* getIp(Client* cli);

/**
 * LISTE
 */

/**
 * Adds a client to the given list
 */
int add_client(Client* cli, Client** list, const unsigned LIST_SIZE);

/**
 * Removes a client from the given list
 */
int remove_client(int clientId, Client** list, const unsigned LIST_SIZE);

#endif // __CLIENT__