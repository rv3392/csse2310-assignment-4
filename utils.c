#include "utils.h"

/* See utils.h */
bool get_line(char** buffer, size_t* capacity, FILE* from) {
    size_t current = 0;
    (*buffer)[0] = '\0';
    int input;
    while (input = fgetc(from), input != EOF && input != '\n') {
        if (current > *capacity - STRING_BUFFER_OFFSET) {
            size_t newCap = (size_t) (*capacity * STRING_RESIZE_MULTIPLIER);
            void* newBlock = realloc(*buffer, newCap);
            if (newBlock == NULL) {
                return false;
            }
            *capacity = newCap;
            *buffer = newBlock;
        }
        (*buffer)[current] = (char)input;
        (*buffer)[++current] = '\0';
    }

    return input != EOF;
}

/* See utils.h */
bool read_message(FILE* from, char* messageBuffer) {
    size_t cap = 80;
    bool hasInput = get_line(&messageBuffer, &cap, from);
    return hasInput;
}

/* See utils.h */
void send_message(FILE* to, char* message) {
    fprintf(to, "%s\n", message);
    fflush(to);
}

/* See utils.h */
bool is_valid_port(char* unparsedPort) {
    for (int i = 0; i < strlen(unparsedPort); i++) {
        if (!isdigit(unparsedPort[i])) {
            return false;
        }
    }

    int parsedPort = strtol(unparsedPort, NULL, BASE_10);
    if (parsedPort < LOW_PORT || parsedPort > HIGH_PORT) {
        return false;
    }

    return true; 
}

/* See utils.h */
bool string_contains_invalid_char(char* checkString) {
    for (int i = 0; i < strlen(checkString); i++) {
        if (checkString[i] == '\n') {
            return true;
        } else if (checkString[i] == '\r') {
            return true;
        } else if (checkString[i] == ':') {
            return true;
        }
    }

    return false;
}
