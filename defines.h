#pragma once
#include <stdio.h>


typedef enum {
    META_COMMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;


/* The buffer we want read */
typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} Input_Buffer;

/* Statements stored here */
typedef struct {
    StatementType type;
} Statement;