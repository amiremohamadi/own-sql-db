#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Print_Prompt() ( printf("db > ") )

/* The buffer we want read */
typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} Input_Buffer;


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

int
main(int argc, char *argv[])
{
    Input_Buffer *input_buffer = new_input_buffer();

    for(;;) {
        Print_Prompt();
        read_input(input_buffer);

        if (strcmp(input_buffer->buffer, ".exit")) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }

    return 0;
}