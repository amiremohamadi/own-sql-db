#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Print_Prompt() ( printf("db > ") )
#define Size_Of_Attribute(struct, attr) (sizeof(((struct *)0)->attr))
#define TABLE_MAX_PAGES 100

typedef enum {
    META_COMMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS
} ExecuteResult;


/* The buffer we want read */
typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} Input_Buffer;

/* Row */
typedef struct {
    int id;
    char username[32];
    char email[255];
} Row;

/* Table */
typedef struct {
    int num_of_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;


/* Statements stored here */
typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

#define ID_SIZE (Size_Of_Attribute(Row, id))
#define USERNAME_SIZE (Size_Of_Attribute(Row, username))
#define EMAIL_SIZE (Size_Of_Attribute(Row, email))
#define ID_OFFSET  0
#define USERNAME_OFFSET (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET (USERNAME_OFFSET + USERNAME_SIZE)
#define ROW_SIZE (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)
#define PAGE_SIZE (4096)
#define ROWS_PER_PAGE (PAGE_SIZE / ROW_SIZE)
#define TABLE_MAX_ROWS (TABLE_MAX_PAGES * ROWS_PER_PAGE)


#define Print_Row(row) ( printf("(%d, %s, %s)\n", row.id, row.username, row.email) )

/* Serilize the row */
void
serialize_row(Row *source, void *destination)
{
    memcpy(destination+ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination+EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
    memcpy(destination+USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
}

/* Desrialize */
void
deserialize_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source+ID_OFFSET, ID_SIZE);
    memcpy(&(destination->email), source+EMAIL_OFFSET, EMAIL_SIZE);
    memcpy(&(destination->username), source+USERNAME_OFFSET, USERNAME_SIZE);
}

/* Find the Slot */
void *
row_slot(Table *table, size_t row_num)
{
    size_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    
    if (page == NULL) {
        // Allocate memory only when we try to access page
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }

    size_t row_offset = row_num % ROWS_PER_PAGE;
    size_t byte_offset = row_offset * ROW_SIZE;

    return page + byte_offset;
}

/* New table */
Table *
new_table()
{
    Table *table = malloc(sizeof(Table));
    table->num_of_rows = 0;

    for (size_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }

    return table;
}

/* Free table */
void
free_table(Table *table)
{
    for (size_t i =0; table->pages[i]; i++) {
        free(table->pages[i]);
    }

    free(table);
}

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
        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s",
                            &statement->row_to_insert.id, &statement->row_to_insert.username,
                            &statement->row_to_insert.email);

        if (args_assigned < 3)
            return PREPARE_SYNTAX_ERROR;

        return PREPARE_SUCCESS;
    }

    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult
execute_insert(Statement *statement, Table *table)
{
    if (table->num_of_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row *row_to_insert = &(statement->row_to_insert);

    serialize_row(row_to_insert, row_slot(table, table->num_of_rows));
    table->num_of_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult
execute_select(Statement *statement, Table *table)
{
    Row row;
    for (size_t i = 0; i < table->num_of_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        Print_Row(row);
    }

    return EXECUTE_SUCCESS;
}

ExecuteResult
execute_statement(Statement *statement, Table *table)
{
    switch (statement->type) {
    case STATEMENT_INSERT:
        return execute_insert(statement, table);
    
        case STATEMENT_SELECT:
            return execute_select(statement, table);
    }
}

int
main(int argc, char *argv[])
{
    Table *table = new_table();
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

        case PREPARE_SYNTAX_ERROR:
            printf("SYNTAX error. Couldn't parse statement.\n");
            continue;
        
        case PREPARE_UNRECOGNIZED_STATEMENT:
            printf("Unrecognized keyword at start of '%s'\n", 
                    input_buffer->buffer);
            continue;
        }

        switch (execute_statement(&statement, table)) {
        case EXECUTE_SUCCESS:
            printf("Executed.\n");
            break;

        case EXECUTE_TABLE_FULL:
            printf("Error. Tabel is full.\n");
            break;
        }
    }


    return 0;
}