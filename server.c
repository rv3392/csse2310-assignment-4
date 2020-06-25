#include "server.h"

/**
 * Sets up a server on an ephemeral port
 * 
 * Both the port and socket are placed into the struct server.
 * If there is any sort of error when setting up the server then
 * ServerError.SERVER_NOT_OK is returned but otherwise ServerError.SERVER_OK
 * is returned.
 * 
 * TODO: This probably needs to be tidied up and generally made better.
 */
ServerError setup_server(Server* server) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;        // IPv4  for generic could use AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // Because we want to bind with it

    int err = getaddrinfo("localhost", 0, &hints, &ai);
    if (err) { // no particular port
        freeaddrinfo(ai);
        return SERVER_NOT_OK;   // could not work out the address
    }
    
        // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (bind(serv, (struct sockaddr*)ai->ai_addr, sizeof(struct sockaddr))) {
        return SERVER_NOT_OK;
    }
    
        // Which port did we get?
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        return SERVER_NOT_OK;
    }

    if (listen(serv, 10)) {     // allow up to 10 connection requests to queue
        return SERVER_NOT_OK;
    }

    server->port = ntohs(ad.sin_port);
    server->socket = serv;

    return SERVER_OK;
}

/**
 * Blocks until a connection is received on the socket stored in the
 * server->socket.
 */
int connection_received(Server* server) {
    return accept(server->socket, 0, 0);
}

/**
 * Handles a connection to the server by starting a pthread.
 * 
 * Starts the function provided as "handler" in a pthread, so as not to
 * block the main process thread with IO processing from the connected client.
 */
ServerError start_connection_handling_thread(
        ConnectionHandler handler, void* data, int connFd) {
    
    ConnectionHandlerArgs* args = calloc(1, sizeof(ConnectionHandlerArgs));
    args->data = data;
    args->connFd = connFd;

    pthread_t tid;
    pthread_create(&tid, NULL, handler, args);

    return SERVER_OK;
}