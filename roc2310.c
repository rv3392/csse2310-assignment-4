#include "roc2310.h"

/**
 * Checks if a mapper2310s port has been provided. 
 * 
 * If a port has been provided i.e. argv[2] != '-' and it is valid i.e. number 
 * between 1 and 65536, then a connection is established to the mapper2310 
 * server. If no port has been provided then the destinations (control2310s) 
 * provided to this plane (roc2310) are checked to make sure they are all 
 * valid ports.
 * 
 * Parameters:
 *  - argc -> the argc of this roc2310 instance
 *  - argv -> the arguments provided to this roc2310 instance
 *  - mapperConnection -> the Client struct representing the connection to the
 *      mapper2310 server.
 * 
 * Returns:
 *  - ROC_OK -> if everything is ok with the mapper and args provided
 *  - ROC_MAPPER_REQUIRED -> if one of the destinations is not a valid port
 *      but no mapper port was provided (i.e. mapper port = "-")
 *  - ROC_INVALID_MAPPER_PORT -> if a port is provided but it is not valid
 *  - ROC_MAPPER_CONN_FAILURE -> if there was an issue connecting to the
 *      given port.
 */
RocError connect_to_mapper(int argc, char** argv, Client* mapperConnection) {
    // TODO: Check if this return order works according to the spec.
    if (strcmp("-", argv[2]) == 0) {
        for (int i = 3; i < argc; i++) {
            if (!is_valid_port(argv[i])) {
                return ROC_MAPPER_REQUIRED;
            }
        }
        return ROC_OK;
    }

    if (!is_valid_port(argv[2])) {
        return ROC_INVALID_MAPPER_PORT;
    }
    int error = setup_client_on_port(argv[2], mapperConnection);
    if (error != CLIENT_OK) {
        return ROC_MAPPER_CONN_FAILURE;
    }

    return ROC_OK;
}

/**
 * Asks a mapper2310 server for the port to a control2310 with a specific "id".
 * 
 * Parameters:
 *  - data -> the data for this roc2310 instance
 *  - id -> the id of the control2310 instance which the mapper is being asked
 *      the port for
 *  - port -> the buffer to write the port for the control2310 into
 * 
 * Returns:
 *  - ROC_MAPPER_NO_ENTRY -> if there is no entry in the mapper2310 for the
 *      given id
 *  - ROC_OK -> if the port was successfully retrieved and written to "port"
 */
RocError ask_for_control_port(Plane* data, char* id, char* port) {
    char* queryBuffer = calloc(strlen(id) + 2, sizeof(char));
    sprintf(queryBuffer, "?%s", id);
    send_message(data->mapperConnection->writeTo, queryBuffer);

    char* responseBuffer = calloc(80, sizeof(char));
    read_message(data->mapperConnection->readFrom, responseBuffer);
    if (strcmp(";", responseBuffer) == 0 || strlen(responseBuffer) == 0) {
        return ROC_MAPPER_NO_ENTRY;
    }
    strcpy(port, responseBuffer);

    free(queryBuffer);
    free(responseBuffer);

    return ROC_OK;
}

/**
 * Convert the destination airport ids (control2310 ids) provided in the args 
 * to ports (control2310 ports).
 * 
 * Parameters:
 *  - argc -> the argc value provided to this roc2310 instance
 *  - argv -> the args provided to this roc2310 instance
 *  - data -> the data of this roc2310 instance
 * 
 * Returns:
 *  - ROC_MAPPER_NO_ENTRY -> if any of the ids provided in the arguments does
 *      not have a valid mapped port in the mapper2310.
 *  - ROC_OK -> if all of the ids are successfully converted.
 */
RocError convert_destination_airports(int argc, char** argv, Plane* data) {
    for (int i = 0; i < data->numDestinations; i++) {
        char* controlPort = argv[i + 3];
        if (!is_valid_port(argv[i + 3])) {
            controlPort = calloc(6, sizeof(char));
            int error = ask_for_control_port(data, argv[i + 3], controlPort);
            if (error != ROC_OK) {
                return error;
            }
        }

        data->destinationPorts[i] = calloc(
                strlen(controlPort) + 1, sizeof(char));
        strcpy(data->destinationPorts[i], controlPort);
    }

    return ROC_OK;
}

/**
 * Visit a destination (control2310) with the given port and send this 
 * roc2310's id to it.
 * 
 * Parameters:
 *  - id -> this roc2310 instance's id
 *  - destinationPort -> the port of the control2310 instance to connect to
 *  - destinationInfo -> a buffer to write the control2310 instance's info to
 * 
 * Returns:
 *  - ROC_CONTROL_CONN_FAILURE -> if connection to the given destinationPort
 *      fails
 *  - ROC_OK -> if the control2310 is successfully connected to, this roc2310's
 *      id is sent and the control2310's info read.
 */
RocError visit_destination(char* id, char* destinationPort,
        char* destinationInfo) {
    Client* destinationConnection = calloc(1, sizeof(Client));
    int error = setup_client_on_port(destinationPort, destinationConnection);
    if (error != CLIENT_OK) {
        return ROC_CONTROL_CONN_FAILURE;
    }

    char* queryBuffer = calloc(strlen(id) + 2, sizeof(char));
    sprintf(queryBuffer, "%s", id);
    send_message(destinationConnection->writeTo, queryBuffer);

    read_message(destinationConnection->readFrom, destinationInfo);

    return ROC_OK;
}

/**
 * Visit all of the destinations (control2310s) loaded by this roc2310.
 * 
 * Attempts to connect to all of the destinations within the plane's data. If
 * any connection fails the roc2310 continues connecting to later control2310
 * ports. After an attempt has been made to connect to all of the ports, this
 * function returns with an error (ROC_CONTROL_CONN_FAILURE).
 * 
 * After connecting to a control2310, its info is requested and added to a log
 * of all control2310 infos within "data".
 * 
 * Parameters:
 *  - data -> this roc2310 instance's data. Also contains an array of
 *      destination ports which are used for to connect to
 * 
 * Returns:
 *  - ROC_CONTROL_CONN_FAILURE -> if connecting to any of the provided
 *      control2310's fails.
 *  - ROC_OK -> if a connection has been made to each provided control2310
 *      port and the communication is successful.
 */
RocError visit_destinations(Plane* data) {
    bool connFailureFlag = false;
    for (int i = 0; i < data->numDestinations; i++) {
        char* destinationInfo = calloc(80, sizeof(char));
        int error = visit_destination(data->id, data->destinationPorts[i], 
                destinationInfo);
        if (error != ROC_OK) {
            connFailureFlag = true;
            continue;
        }

        add_list_item(data->visitedAirportInfos, destinationInfo);
    }

    if (connFailureFlag == true) {
        return ROC_CONTROL_CONN_FAILURE;
    }

    return ROC_OK;
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
int visited_airport_info_to_string(char* buffer, size_t capacity, 
        void* toConvert) {
    return snprintf(buffer, capacity, "%s", (char*) toConvert);
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
int visited_airport_info_compare(const void* item1, const void* item2) {
    return strcmp(*(char* const*) item1, *(char* const*) item2);
}

/**
 * Prints the infos for all of the airports (control2310s) visited by this
 * roc2310. As stored in data.
 * 
 * Parameters:
 *  - data -> this roc2310 instance's data
 */
void print_airport_infos(Plane* data) {
    char* buffer = calloc(1, sizeof(char));
    size_t capacity = 1;
    get_list_as_str(data->visitedAirportInfos, &buffer, &capacity);

    if (strlen(buffer) > 0) {
        send_message(stdout, buffer);
    }
}

/**
 * roc2310.
 * 
 * Starts a roc2310 instance, connects to the mapper2310's provided port and
 * converts all of the control2310 provided ports. Then, connects to each
 * provided control2310 and prints their "info" strings.
 */
int main(int argc, char** argv) {
    int error = ROC_OK;

    if (argc < 3) {
        handle_roc_error(ROC_INVALID_NUM_ARGS);
    }

    Client* mapperConnection = calloc(1, sizeof(Client));
    error = connect_to_mapper(argc, argv, mapperConnection);
    if (error != ROC_OK) {
        handle_roc_error(error);
    }

    Plane* data = calloc(1, sizeof(Plane));
    data->id = argv[1];
    data->numDestinations = argc - 3;
    data->destinationPorts = calloc(data->numDestinations, sizeof(char*));
    data->mapperConnection = mapperConnection;

    data->visitedAirportInfos = calloc(1, sizeof(List));
    create_list(data->visitedAirportInfos, sizeof(VisitedAirportInfo),
            visited_airport_info_to_string, visited_airport_info_compare);

    error = convert_destination_airports(argc, argv, data);
    if (error != ROC_OK) {
        handle_roc_error(ROC_MAPPER_NO_ENTRY);
    }

    // Exit with error after printing a log of the destinations visited if
    // an error is encountered in visit_destinations.
    // This is so that the roc continues to visit destinations even if
    // connecting to one of the provided control2310s fails.
    error = visit_destinations(data);
    print_airport_infos(data);

    if (error != ROC_OK) {
        handle_roc_error(ROC_CONTROL_CONN_FAILURE);
    }

    return ROC_OK;
}