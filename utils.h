#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* The lowest valid port */
#define LOW_PORT 1
/* The highest valid port */
#define HIGH_PORT 65536
/* strtol should use base 10 */
#define BASE_10 10

/* Offset for get_line checking when it is about to reach buffer capacity */
#define STRING_BUFFER_OFFSET 2
/* How much to resize the buffer by when it reaches capacity */
#define STRING_RESIZE_MULTIPLIER 1.5

/**
 * Gets a line from the provided file and writes it to the provided buffer
 * resizing the buffer as necessary.
 * 
 * Parameters:
 *  - buffer -> buffer to write single line of input to
 *  - capacity -> capacity of the buffer
 *  - from -> file to read input from
 * 
 * Returns:
 *  - true -> if there is more input to read
 *  - false -> if eof has been reached
 */
bool get_line(char** buffer, size_t* capacity, FILE* from);

/**
 * Reads a message from the provided file.
 * 
 * Parameters:
 *  - from -> file to read input from
 *  - messageBuffer -> buffer to write single line of input to
 */
bool read_message(FILE* from, char* messageBuffer);

/**
 * Sends a message to the provided file and also flushes that file so that
 * messages aren't stuck in pipes.
 * 
 * Parameters:
 *  - to -> file to write output to
 *  - message -> the buffer to write to the output file
 */
void send_message(FILE* to, char* message);

/**
 * Checks if the given port is a valid port. That is, unparsedPort > 0 
 * and <= 65536, and unparsedPort does not contain any non-numerical 
 * characters.
 * 
 * Returns true if it is valid and false if it isn't.
 */
bool is_valid_port(char* unparsedPort);

/**
 * Checks if the provided "checkString" contains any characters that are
 * invalid for airport IDs or airport info strings. These characters are:
 * '\n', '\r' and ':'. All other characters are valid.
 * 
 * Parameters:
 *  - checkString -> the string to check
 * 
 * Returns:
 * - true -> if the string contains any invalid characters.
 * - false -> if the string does not contain any invalid characters.
 */
bool string_contains_invalid_char(char* checkString);

#endif