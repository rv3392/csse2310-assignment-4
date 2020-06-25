#include "error.h"

/* See error.h */
void handle_mapper_error(MapperError errorCode) {
    char* errorMessage = "";
    switch (errorCode) {
        case MAPPER_OK:
            return;
        case MAPPER_ERROR:
            errorMessage = "Big oh no?";
            break;
    }

    fprintf(stderr, "%s", errorMessage);
    fflush(stderr);

    exit(errorCode);
}

/* See error.h */
void handle_control_error(ControlError errorCode) {
    char* errorMessage = "";
    switch (errorCode) {
        case CONTROL_OK:
            return;
        case CONTROL_INVALID_NUM_ARGS:
            errorMessage = "Usage: control2310 id info [mapper]";
            break;
        case CONTROL_INVALID_ARGS:
            errorMessage = "Invalid char in parameter";
            break;
        case CONTROL_INVALID_PORT:
            errorMessage = "Invalid port";
            break;
        case CONTROL_INVALID_MAPPER:
            errorMessage = "Can not connect to map";
            break;
    }

    fprintf(stderr, "%s\n", errorMessage);
    fflush(stderr);

    exit(errorCode);
}

/* See error.h */
void handle_roc_error(RocError errorCode) {
    char* errorMessage = "";
    switch (errorCode) {
        case ROC_OK:
            return;
        case ROC_INVALID_NUM_ARGS:
            errorMessage = "Usage: roc2310 id mapper {airports}";
            break;
        case ROC_INVALID_MAPPER_PORT:
            errorMessage = "Invalid mapper port";
            break;
        case ROC_MAPPER_REQUIRED:
            errorMessage = "Mapper required";
            break;
        case ROC_MAPPER_CONN_FAILURE:
            errorMessage = "Failed to connect to mapper";
            break;
        case ROC_MAPPER_NO_ENTRY:
            errorMessage = "No map entry for destination";
            break;
        case ROC_CONTROL_CONN_FAILURE:
            errorMessage = "Failed to connect to at least one destination";
            break;
    }

    fprintf(stderr, "%s\n", errorMessage);
    fflush(stderr);

    exit(errorCode);
}