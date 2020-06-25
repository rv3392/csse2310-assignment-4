#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

/**
 * A Client struct is used to store details of a connected port.
 * Members:
 *  - readFrom -> the FILE* to read input from the server from
 *  - writeTo -> the FILE* to write output to the server to
 *  - socket -> the file descriptor (fd) for the client's scocket
 */
struct Client {
    FILE* readFrom;
    FILE* writeTo;
    int socket;
};

/* ClientError is an enum of Client-related errors */
enum ClientError {
    CLIENT_OK,
    CLIENT_NOT_OK
};

typedef struct Client Client;
typedef enum ClientError ClientError;

/* See client.c */
ClientError setup_client_on_port(char* port, Client* client);

#endif