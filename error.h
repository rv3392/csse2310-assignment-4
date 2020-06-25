#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>

/* The error codes for mapper2310 related errors */
enum MapperError {
    MAPPER_OK,
    MAPPER_ERROR
};

/* The error codes for control2310 related errors */
enum ControlError {
    CONTROL_OK,
    CONTROL_INVALID_NUM_ARGS,
    CONTROL_INVALID_ARGS,
    CONTROL_INVALID_PORT,
    CONTROL_INVALID_MAPPER
};

/* The error codes for roc2310 related errors */
enum RocError {
    ROC_OK,
    ROC_INVALID_NUM_ARGS,
    ROC_INVALID_MAPPER_PORT,
    ROC_MAPPER_REQUIRED,
    ROC_MAPPER_CONN_FAILURE,
    ROC_MAPPER_NO_ENTRY,
    ROC_CONTROL_CONN_FAILURE
};

typedef enum MapperError MapperError;
typedef enum ControlError ControlError;
typedef enum RocError RocError;

/**
 * Prints an error message (for mapper2310 instances) to stderr and exits.
 * 
 * Parameter:
 *  - errorCode -> the error being handled
 */
void handle_mapper_error(MapperError errorCode);

/**
 * Prints an error message (for control2310 instances) to stderr and exits.
 * 
 * Parameter:
 *  - errorCode -> the error being handled
 */
void handle_control_error(ControlError errorCode);

/**
 * Prints an error message (for roc2310 instances) to stderr and exits.
 * 
 * Parameter:
 *  - errorCode -> the error being handled
 */
void handle_roc_error(RocError errorCode);

#endif