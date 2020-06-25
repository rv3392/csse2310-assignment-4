#ifndef ROC_2310_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"
#include "client.h"
#include "list.h"
#include "utils.h"

typedef struct Plane Plane;
typedef char* VisitedAirportInfo;

/**
 * A struct to store all of the data for a roc2310 instance.
 * Members:
 *  - id -> the id of this roc2310 instance
 *  - numDestinations -> the total number of control2310s that this roc2310
 *      has to visit/connect to
 *  - destinationPorts -> the ports for all of the control2310s this roc2310
 *      has to connect to
 *  - mapperConnection -> contains the file descriptors and files after
 *      a connection to a mapper2310 is made.
 *  - visitedAirportInfos -> a list of all visited airports' info strings.
 *      That is, a list of all control2310s' infos that were connected to.
 */
struct Plane {
    char* id;
    int numDestinations;
    char** destinationPorts;
    Client* mapperConnection;
    List* visitedAirportInfos;
};

#endif