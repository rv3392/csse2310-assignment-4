#ifndef CONTROL_2310_H
#define CONTROL_2310_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "error.h"
#include "server.h"
#include "client.h"
#include "list.h"
#include "utils.h"

typedef struct Airport Airport;
typedef char* VisitingPlaneName;

/**
 * A struct to store all of the data for a control2310 instance.
 * Members:
 *  - id -> the id of this control2310
 *  - info -> the info string of this control2310
 *  - visitingPlaneNames -> a list of VisitingPlaneName. i.e. a list of
 *      strings of the ids of the roc2310s that have connected to this
 *      control2310.
 *  - mapperPort -> the port of the mapper2310 this control2310 should connect
 *      to
 */
struct Airport {
    char* id;
    char* info;
    List* visitingPlaneNames;
    int mapperPort;
};

#endif