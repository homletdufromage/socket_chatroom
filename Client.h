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
 * Ajoute un client à la liste donnée
 */
int add_client(Client* cli, Client** list, const unsigned LIST_SIZE);

/**
 * Enlève un client de la liste donnée
 */
int remove_client(int clientId, Client** list, const unsigned LIST_SIZE);

#endif // __CLIENT__