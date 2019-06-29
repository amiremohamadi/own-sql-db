#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"

#define Print_Prompt() ( printf("db > ") )


/* Create a new buffer object */
Input_Buffer *
new_input_buffer()
{
    Input_Buffer *input_buffer = malloc(sizeof(Input_Buffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void
read_input(Input_Buffer *input_buffer)
{
    ssize_t bytes_read = getline(&(input_buffer->buffer),
                                &(input_buffer->buffer_length), stdin);
    
    if (bytes_read <= 0) {
        printf("Error reading input.\n");
        exit(EXIT_FAILURE);
    }

    // Ignoring new line (\n)
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void
close_input_buffer(Input_Buffer *input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandResult
do_meta_command(Input_Buffer *input_buffer)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }

    return META_COMMAND_UNRECOGNIZED;
}

PrepareResult
prepare_statement(Input_Buffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }

    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void
execute_statement(Statement *statement)
{
    switch (statement->type) {
    case STATEMENT_INSERT:
        printf("This is insert command\n");
        break;
    
        case STATEMENT_SELECT:
        printf("This is select command\n");
        break;
    }
}

int
main(int argc, char *argv[])
{
    Input_Buffer *input_buffer = new_input_buffer();

    for(;;) {
        Print_Prompt();
        read_input(input_buffer);

        // Check for meta-command
        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
            case META_COMMAND_UNRECOGNIZED:
                printf("Unrecognized command '%s'\n", input_buffer->buffer);
                continue;

            default:
                continue;
            }
        }

        // Check for a real command
        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
        case PREPARE_SUCCESS:
            break;
        
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("Unrecognized keyword at start of '%s'\n", 
                    input_buffer->buffer);
            continue;
        }

        execute_statement(&statement);
        printf("Executed.\n");
    }


    return 0;
}