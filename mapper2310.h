#ifndef MAPPER_2310_H
#define MAPPER_2310_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "error.h"
#include "server.h"
#include "list.h"

typedef struct MapperData MapperData;
typedef struct Mapper Mapper;
typedef struct MappedAirport MappedAirport;

/**
 * A struct to store a mapping between a control2310 id and its port.
 * Members:
 *  - id -> the id of the control2310 being stored
 *  - port -> the port that the control2310 is listening on
 */
struct MappedAirport {
    char* id;
    int port;
};

/**
 * A struct to store all of the data related to a mapper2310 instance for 
 * access to within the a mapper2310 isntance.
 * Members:
 *  - airports -> a list of MappedAirports
 *  - port -> the port that mapper2310 is listening on
 */
struct Mapper {
    List* airports;
    int port;
};

#endif