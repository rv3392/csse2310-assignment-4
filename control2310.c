#include "control2310.h"

/**
 * Handles input from a "client" (generally a roc2310) connected to the port.
 * 
 * If the received input is:
 *  - "log" -> print the log of all plane (roc2310) id's that have visited
 *      this control2310 to "to". This log is located in "data".
 *  - anything else -> add the provided input to the log of plane's that have
 *      visited.
 * 
 * Parameters:
 *  - to -> the file to write output to
 *  - message -> the input from the connected client
 *  - data -> the data for this control2310 instance
 */
void handle_client_command(FILE* to, char* message, Airport* data) {
    if (string_contains_invalid_char(message)) {
        return;
    }

    if (strcmp("log", message) == 0) {
        char* planesAsString = calloc(1, sizeof(char));
        size_t capacity = 1;

        sort_list(data->visitingPlaneNames);
        get_list_as_str(data->visitingPlaneNames, &planesAsString, &capacity);

        if (strlen(planesAsString) > 0) {
            send_message(to, planesAsString);
        }
        send_message(to, ".");
    } else {
        VisitingPlaneName name = calloc(strlen(message) + 1, sizeof(char));
        strcpy(name, message);

        add_list_item(data->visitingPlaneNames, name);
        send_message(to, data->info);
    }
}

/**
 * Handler for connections to this control2310 instance. Takes the arg
 * Made to be called as a function pointer in order to start a new thread.
 * 
 * Parameters:
 *  - uncastedArgs -> this struct is casted to a ConnectionHandlerArgs which
 *      contains an Airport* struct for the data and a socketFd for the
 *      connected roc2310.
 * 
 * Return:
 *  NULL - if the connection is ended successfully.
 */
void* handle_control_connection(void* uncastedArgs) {
    // Create a FILE* to read and write from the connected client
    ConnectionHandlerArgs* args = (ConnectionHandlerArgs*) uncastedArgs;
    FILE* readFrom = fdopen(dup(args->connFd), "r");
    FILE* writeTo = fdopen(args->connFd, "w");

    // Read any commands from the client
    char* commandMessage = calloc(80, sizeof(char));
    while(read_message(readFrom, commandMessage)) {
        Airport* commandData = (Airport*) args->data;
        handle_client_command(writeTo, commandMessage, commandData);
    }

    fclose(readFrom);
    fclose(writeTo);
    return NULL;
}

/**
 * A wrapper around strcmp. 
 * 
 * For use with searching and sorting List structs of type VisitedAirportInfo
 * (aka char*).
 * 
 * Parameters:
 *  - item1 -> the first item to compare
 *  - item2 -> the second item to compare
 * 
 * Returns:
 *  - negative integer -> if item1 < item2
 *  - zero -> if item1 == item2
 *  - positive integer -> if item1 > item2
 */
int visiting_plane_name_compare(const void* item1, const void* item2) {
    return strcmp(*(char* const*) item1, *(char* const*) item2);
}

/**
 * A wrapper around snprintf. 
 * 
 * For use with printing List structs of type VisitedAirportInfo (aka char*).
 * 
 * Parameters:
 *  - buffer -> the buffer to write to using snprintf
 *  - capacity -> the size of the buffer
 *  - toConvert -> the item to be converted to a string
 * 
 * Returns:
 *  - The output of snprintf. Which is the number of characters written to the
 *      buffer excluding the null terminator. If the number of characters to be
 *      written is greater than the capacity then, the write is truncated and
 *      the number of characters that would have been written is returned.
 */
int visiting_plane_name_to_string(char* buffer, size_t capacity,
        ListItem toConvert) {
    return snprintf(buffer, capacity, "%s", (char*) toConvert);
}

/**
 * Sets up this control2310 instance's data struct using the argv arguments.
 * Parses each argument and checks that it follows the provided conventions
 * before appropriately storing it into the data struct for later use.
 * 
 * Parameters:
 *  - data -> the struct to store the data into
 *  - argv -> the args provided to this control2310 instance
 * 
 * Returns:
 *  - CONTROL_OK -> if all arguments are read and parsed successfully.
 *  - CONTROL_INVALID_ARGS -> if any of the args is invalid as per the
 *      assignment spec.
 */
ControlError setup_control_data(Airport* data, char** argv) {
    if (string_contains_invalid_char(argv[1])) {
        return CONTROL_INVALID_ARGS;
    }
    data->id = calloc(strlen(argv[1]) + 1, sizeof(char));
    strcpy(data->id, argv[1]);


    if (string_contains_invalid_char(argv[2])) {
        return CONTROL_INVALID_ARGS;
    }
    data->info = calloc(strlen(argv[2]) + 1, sizeof(char));
    strcpy(data->info, argv[2]);

    data->visitingPlaneNames = calloc(1, sizeof(List));
    create_list(data->visitingPlaneNames, sizeof(VisitingPlaneName), 
            visiting_plane_name_to_string, visiting_plane_name_compare); 

    return CONTROL_OK;
}

/**
 * Connects to the mapper2310 instance at the provided port and sends this
 * control2310 instance's control id and port as a formated !ID:PORT command.
 * After this, it disconnects from the mapper.
 * 
 * Parameters:
 *  - controlPort -> the string version of the port to send to the mapper
 *  - mapperPort -> the port to connect to the mapper
 *  - data -> further data including the id that is required to connect to the
 *      mapper and send it messages.
 * 
 * Returns:
 *  - CONTROL_OK -> if the mapper was successfully connected to and the
 *      details of this control2310 instance were sent.
 *  - CONTROL_INVALID_MAPPER -> if an attempt was made to connect to the
 *      mapper2310 instance, however failed from a port that isn't used
 *      for a mapper2310 instance.
 */
ControlError register_with_mapper(char* controlPort, char* mapperPort,
        Airport* data) {
    ControlError error = CONTROL_OK;
    Client* client = calloc(1, sizeof(Client));
    error = (ControlError) setup_client_on_port(mapperPort, client);
    if (error != CONTROL_OK) {
        return CONTROL_INVALID_MAPPER;
    }

    char* messageBuffer = calloc(strlen(data->id) + 
            strlen(controlPort) + 3, sizeof(char));
    sprintf(messageBuffer, "!%s:%s", data->id, controlPort);
    send_message(client->writeTo, messageBuffer);

    shutdown(client->socket, 2);
    free(client);

    return CONTROL_OK;
}

/**
 * control2310.
 * 
 * Connects to the provided mapper2310 port, if provided, and sends this
 * control2310 instance's details. Then sets up a control2310 server for 
 * roc2310 instance's to connect to. Once a connection is detected, it is
 * passed to a thread and handled seperately from the main thread.
 * 
 * For exit conditions, see error.c.
 */
int main(int argc, char** argv) {
    int error;

    if (argc != 3 && argc != 4) {
        handle_control_error(CONTROL_INVALID_NUM_ARGS);
    }

    // Creates a struct to store this airports id, info, port and mapper_port
    Airport* data = calloc(1, sizeof(Airport));
    error = setup_control_data(data, argv);
    if (error != CONTROL_OK) {
        handle_control_error(error);
    }

    // Start a server for clients (in this case rocs/planes) to connect to
    Server* control = calloc(1, sizeof(Server));
    error = setup_server(control);
    if (error != SERVER_OK) {
        handle_control_error(CONTROL_OK);
    }

    char port[6];
    sprintf(port, "%d", control->port);

    if (argc == 4) {
        if (!is_valid_port(argv[3])) {
            handle_control_error(CONTROL_INVALID_PORT);
        }
    }

    fprintf(stdout, "%d\n", control->port);
    fflush(stdout);

    // If a mapper port has been provided
    if (argc == 4) {
        error = register_with_mapper(port, argv[3], data);
        if (error != CONTROL_OK) {
            handle_control_error(error);
        }
    }
    
    // Wait for any connections and handle on another thread.
    int connFd;
    while (connFd = connection_received(control), connFd >= 0) {
        error = start_connection_handling_thread(
                handle_control_connection, data, connFd);
    }

    return CONTROL_OK;
}
