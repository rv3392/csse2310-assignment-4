#include "client.h"

/**
 * Resolves the port provided. This function assumes that any port provided
 * is trying to connect to a localhost.
 * 
 * Parameters:
 *  - ai -> address info to use to resolve the host a
 *  - port -> port to resolve
 * 
 * Returns:
 *  - CLIENT_OK -> if the port was successfully resolved
 *  - CLIENT_NOT_OK -> if the port was not successfully resolved
 */
ClientError resolve_port(struct addrinfo** ai, char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error = getaddrinfo("localhost", port, &hints, ai);
    if (error != CLIENT_OK) {
        return CLIENT_NOT_OK;   // could not work out the address
    }

    return CLIENT_OK;
}

/**
 * Creates a connection with the port and stores the socket's file descriptor
 * into socketFd.
 * 
 * Parameters:
 *  - ai -> the resolved address to connect to
 *  - socketFd -> a pointer to memory where the socketFd can be stored.
 * 
 * Returns:
 *  - CLIENT_OK -> if the address is successfully connected to
 *  - CLIENT NOT_OK -> if the address is not successfully connected to
 */
ClientError connect_to_port(struct addrinfo* ai, int* socketFd) {
    *socketFd = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    int error = connect(*socketFd, (struct sockaddr*)ai->ai_addr,
            sizeof(struct sockaddr));
    if (error != CLIENT_OK) {
        return CLIENT_NOT_OK;
    }

    return CLIENT_OK;
}

/**
 * Open separate files for reading and writing from the file descriptor of the
 * connected socket. This is used so that the reads and writes are not
 * happening to the same file and conflicitng with each other.
 * 
 * Parameters:
 *  - client -> the client to store the opened FILE*'s to
 *  - socketFd -> the file descriptor for the socket trying to be opened
 */
void open_read_write_files(Client* client, int socketFd) {
    FILE* writeTo = fdopen(dup(socketFd), "w");
    FILE* readFrom = fdopen(socketFd, "r");

    client->writeTo = writeTo;
    client->readFrom = readFrom;
    client->socket = socketFd;
}

/**
 * Creates a Client that is connected to the given port and waiting for any
 * input or output on the created FILE*'s.
 * 
 * First resolves the provide port on the "localhost" and then connects to
 * the port if that is successful. Finally, the socket file descriptor is
 * opened and the function returns.
 * 
 * Parameters:
 *  - client -> client to store FILE* and socket information to
 *  - port -> port to connect the client to
 * 
 * Returns:
 *  - CLIENT_OK -> if the connection occurs successfully
 *  - CLIENT_NOT_OK -> if the connection fails to resolve the host at the port
 *      or if the connection to the resolved host fails.
 */
ClientError setup_client_on_port(char* port, Client* client) {
    int error = CLIENT_OK;
    struct addrinfo* ai = 0;
    error = resolve_port(&ai, port);
    if (error != CLIENT_OK) {
        return CLIENT_NOT_OK;
    }

    int socketFd = 0;
    error = connect_to_port(ai, &socketFd);
    if (error != CLIENT_OK) {
        return CLIENT_NOT_OK;
    }

    open_read_write_files(client, socketFd);

    return CLIENT_OK;
}
