#ifndef PARALLEL_SERVER_H

#include <pthread.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

enum ServerError {
    SERVER_OK,
    SERVER_NOT_OK
};
typedef enum ServerError ServerError;

struct Server {
    int socket;
    int port;
};
typedef struct Server Server;

struct ConnectionHandlerArgs {
    int connFd;
    void* data;
};
typedef struct ConnectionHandlerArgs ConnectionHandlerArgs;

/**
 * A ClientHandler takes a char* message to update and a void* struct 
 * to update using this message. It should return a server error.
 */
typedef void* (*ConnectionHandler)(void*);

ServerError setup_server(Server* server);
int connection_received(Server* server);
ServerError start_connection_handling_thread(
        ConnectionHandler handler, void* data, int connFd);

#endif