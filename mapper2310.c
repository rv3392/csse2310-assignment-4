#include "mapper2310.h"

/**
 * Gets the port for the given airport id from the list of mapped airports.
 * 
 * Parameters:
 *  - id -> the id of the airport to get the port for
 *  - buffer -> buffer to write the string version of the port to
 *  - data -> data for this instance of mapper2310
 * 
 * Returns:
 *  - SERVER_OK
 */
ServerError get_airport_port(char* id, char* buffer, Mapper* data) {
    // Create a MappedAirport to use to search the MappedAirport List with,
    // this airport uses "id" as its id
    MappedAirport* searchAirport = calloc(1, sizeof(MappedAirport));
    searchAirport->id = calloc(strlen(id) + 1, sizeof(char));
    memcpy(searchAirport->id, id, strlen(id) + 1);
    searchAirport->port = -1;

    void** foundAirport = calloc(1, sizeof(void*));
    *foundAirport = searchAirport;
    search_list(data->airports, &searchAirport, foundAirport);

    if (((MappedAirport*) *foundAirport)->port == -1) {
        strcpy(buffer, ";");
    } else {
        sprintf(buffer, "%d", ((MappedAirport*) *foundAirport)->port);
    }

    free(searchAirport);
    return SERVER_OK;
}

/**
 * Parses a message of the format ID:PORT into a MappedAirport struct.
 * 
 * Parameters:
 *  - message -> the "ID:PORT" string
 *  - mappingBuffer -> the MappedAirport to write the mapping to
 * 
 * Returns:
 *  - SERVER_OK -> if the function completes as expected
 *  - SERVER_NOT_OK -> if no port is provided or the port is invalid in anyway.
 *      A port is invalid if it contains any non-numeric characters or its
 *      value is not >= 1 and <= 65536.
 */
ServerError parse_new_airport(char* message, MappedAirport* mappingBuffer) {
    int colonCount = 0;
    for (int i = 0; i < strlen(message); i++) {
        if (message[i] == ':') {
            colonCount++;
        }
    }
    if (colonCount > 1) {
        return SERVER_NOT_OK;
    }

    // Split ID:PORT into id and port
    char* leftOver;
    char* tempId = strtok_r(message, ":", &leftOver);
    char* id = calloc(strlen(tempId) + 1, sizeof(char));
    strcpy(id, tempId);

    char* unparsedPort = strtok_r(leftOver, ":", &leftOver);

    if (unparsedPort == NULL) {
        return SERVER_NOT_OK;
    }
    for (int i = 0; i < strlen(unparsedPort); i++) {
        if (!isdigit(unparsedPort[i])) {
            return SERVER_NOT_OK;
        }
    }

    int port = strtol(unparsedPort, NULL, 10);
    if (port < 1 || port > 65536) {
        return SERVER_NOT_OK;
    }

    mappingBuffer->id = id;
    mappingBuffer->port = port;

    return SERVER_OK;
}

/**
 * Handles an add command from the client. That is a command of the format
 * "!ID:PORT". If the ID doesn't already exist in this mapper2310 then it
 * is added to the mapper along with its PORT.
 * 
 * If an error is encountered the function returns early rather than returning
 * an error.
 * 
 * Parameters:
 *  - data -> the mapper2310 data to use to run this command
 *  - message -> the command received in its entirety (including the '!')
 */
void handle_add_command(Mapper* data, char* message) {
    MappedAirport* airport = calloc(1, sizeof(MappedAirport));
    int errorCode = parse_new_airport(message + 1, airport);
    if (errorCode != SERVER_OK) {
        // Supress any errors with executing the command and continue
        return;
    }

    // If the airport id already exists within the list then ignore this
    // new one being added
    void** foundAirport = calloc(1, sizeof(void*));
    *foundAirport = airport;
    search_list(data->airports, &airport, foundAirport);
    if ((MappedAirport*) *foundAirport != airport) {
        return;
    }

    add_list_item(data->airports, airport);
}

/**
 * Handles a search command from the client. That is, a command of the format
 * '?ID'. If the ID is not in the list of mapped control's then a ';' is
 * written to the 'to' file.
 * 
 * Parameters:
 *  - data -> the mapper2310 data to use to run this command
 *  - message -> the command received in its entirety (including the '?')
 *  - to -> the file to write the output of this command to
 */
void handle_search_command(Mapper* data, char* message, FILE* to) {
    char* stringPort = calloc(6, sizeof(char));
    get_airport_port(message + 1, stringPort, data);
    send_message(to, stringPort);
}

/**
 * Handles a print command from the client. That is, a command of the format
 * '@'. Uses the MappedAirport List's string conversion function to print
 * each line with a newline in between.
 * 
 * If an error occurs while mapper2310 is doing this, it returns early but
 * does not throw any errors.
 * 
 * Parameters:
 *  - data -> the mapper2310 data to use to run this command
 *  - to -> the file to write the output of this command to
 */
void handle_print_command(Mapper* data, FILE* to) {
    char* stringAirports = calloc(1, sizeof(char));
    size_t capacity = 1;

    sort_list(data->airports);
    get_list_as_str(data->airports, &stringAirports, &capacity);
    if (strlen(stringAirports) != 0) {
        send_message(to, stringAirports);
    }

    free(stringAirports);
}

/**
 * Handles any command read from a client.
 * 
 * If any sort of error is encountered while processing a command, it is
 * quitely ignored and the server continues to wait for new input.
 * 
 * There are 3 types of commands denoted by the character they begin with.
 * These are:
 *  - '?' -> "?ID" -> Get the Port for the airport with the provided ID
 *  - '!' -> '!ID:PORT' -> Add the ID and PORT for the aiport to this mapper.
 *  - '@' -> Print all IDs and Ports stored in this mapper.
 * 
 * Parameters:
 *  - to -> the file to write the output of any command to
 *  - message -> the command received from the mapper's input
 *  - data -> this mapper instance's data
 */
void handle_client_command(FILE* to, char* message, Mapper* data) {
    // The command is represented by the first character of the command so
    // just message[0] can be checked to see what command has been requested.
    switch (message[0]) {
        case '?':
            handle_search_command(data, message, to);
            break;
        case '!':
            handle_add_command(data, message);
            break;
        case '@':
            handle_print_command(data, to);
            break;
    }
}

/**
 * Handles a connection to the mapper from a client.
 * 
 * Takes any input read from the client connected on the server's port and
 * then parses each of the inputs. The function returns a SERVER_NOT_OK if 
 * anything fails when parsing, which is later handled further downstream.
 * 
 * Updates data with any new airport ID:Name.
 * 
 * TODO: A void* parameter is kind of ugly
 */
void* handle_mapper_connection(void* uncastedArgs) {
    // Create a FILE* to read and write from the connected client
    ConnectionHandlerArgs* args = (ConnectionHandlerArgs*) uncastedArgs;
    FILE* readFrom = fdopen(dup(args->connFd), "r");
    FILE* writeTo = fdopen(args->connFd, "w");

    // Read any commands from the client
    char* commandMessage = calloc(80, sizeof(char));
    while(read_message(readFrom, commandMessage)) {
        Mapper* commandData = (Mapper*) args->data;
        handle_client_command(writeTo, commandMessage, commandData);
    }

    fclose(readFrom);
    fclose(writeTo);
    return NULL;
}

/**
 * Wrapper around snprintf to use when converting a MappedAirport to string.
 * 
 * Parameters:
 *  - buffer -> buffer to write the string version to
 *  - capacity -> the size of "buffer"
 *  - toConvert -> a pointer to the MappedAirport struct to convert to a string
 * 
 * Returns:
 *  - The number of characters written by snprintf to "buffer"
 */
int mapped_airport_to_string(char* buffer, size_t capacity,
        void* toConvert) {
    MappedAirport* airport = (MappedAirport*) toConvert;
    return snprintf(buffer, capacity, "%s:%d", airport->id, airport->port);
}

/**
 * Wrapper around strcmp to use when comparing two MappedAirport structs.
 * 
 * Parameters:
 *  - item1 -> First MappedAirport to compare
 *  - item2 -> Second MappedAirport to compare
 * 
 * Returns:
 *  - A negative value -> if item1's id < item2's id, that is item1's is comes 
 *      before item2's in alphabetic order.
 *  - Zero -> if item1 = item2, that is item1's id and item2's id are exactly 
 *      the same.
 *  - A positive value -> if item1's id > item2's id, that is item1's id comes
 *      after item2's id in alphabetic order.
 */
int mapped_airport_compare(const void* item1, const void* item2) {
    const MappedAirport* airport1 = *(MappedAirport**) item1;
    const MappedAirport* airport2 = *(MappedAirport**) item2;
    return strcmp(airport1->id, airport2->id);
}

/**
 * Setup this instance of mapper2310.
 * 
 * Sets up the List used to store the mappings between airport id's and their
 * respective ports. Creates a Server for roc2310 and control2310 instances
 * to connect through and prints the port to stdout.
 * 
 * Parameters:
 *  - data -> the struct to store this mapper2310 instance's data into
 * 
 * Returns:
 *  - one of the MapperErrors if any sort of problem is encountered.
 *  - Otherwise MapperError.MAPPER_OK is returned.
 */
MapperError setup_mapper(Mapper* data, Server* server) {
    data->airports = calloc(1, sizeof(List));
    create_list(data->airports, sizeof(MappedAirport*), 
            mapped_airport_to_string, mapped_airport_compare);

    int errorCode = setup_server(server);
    if (errorCode != SERVER_OK) {
        return MAPPER_ERROR;
    }
    fprintf(stdout, "%d\n", server->port);
    fflush(stdout);

    return MAPPER_OK;
}

/**
 * mapper2310.
 * 
 * Starts a mapper2310 server and waits for and then handles connections.
 */
int main(int argc, char** argv) {
    int errorCode = MAPPER_OK;

    if (argc != 1) {
        handle_mapper_error(MAPPER_ERROR);
    }

    Mapper* data = calloc(1, sizeof(Mapper));
    Server* mappingServer = calloc(1, sizeof(Server));
    errorCode = setup_mapper(data, mappingServer);
    if (errorCode != MAPPER_OK) {
        handle_mapper_error(MAPPER_ERROR);
    } 
    
    // Wait for any connections and handle on another thread.
    int connFd;
    while (connFd = connection_received(mappingServer), connFd >= 0) {
        errorCode = start_connection_handling_thread(
                handle_mapper_connection, data, connFd);
    }

    return 0;
}
