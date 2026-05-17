
#include "buffer.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>



/*
static
void print_buffer_linked_list(Buffer* buf) {
    
    printf("\n");
    printf("##### %s #####\n\n", __func__);

    for(size_t i = 0; i < buf->num_row_nodes; i++) {
        Bufrow* row = &buf->rows[i];

        printf("Row --- [\033[34m%p\033[0m] %li\n", row, i);
        printf("   Prev: \033[35m%p\033[0m\n", row->prev);
        printf("   Next: \033[35m%p\033[0m\n", row->next);
        printf("\n");

    }

    printf("#############################\n\n");
}


static
void validate_linked_list(Buffer* buf) {
    bool error = false;
    Bufrow* row = buf->rows_head;
    while(row) {

        printf("[\033[34m%p\033[0m] ", row);
        bool tailorhead = false;
        if(row->prev == NULL) {
            printf("HEAD");
            tailorhead = true;
        }
        else {
            if(row->prev->next != row) {
                error = true;
                break;
            }
        }

        if(row->next == NULL) {
            printf("TAIL");
            tailorhead = true;
        }
        else {
            if(row->next->prev != row) {
                error = true;
                break;
            }

        }

        if(!tailorhead) {
            printf("....");
        }


        printf(" | ");

        if(row->data) {
            printf("%s", row->data);
        }

        printf("\n");
        row = row->next;
    }
    
    if(error) {
        printf("\033[1;31m Corrupted linked list\033[0m\n");
    }
    else {
        printf("\033[1;32m + Linked list validated\033[0m\n");
    }
    printf("\n");
}
*/

Buffer* buffer_allocate() {
    Buffer* buf = malloc(sizeof *buf);
    buf->rows = NULL;
    buf->rows_head = NULL;
    buf->num_row_nodes = 0;
    buf->num_row_nodes_alloc = 0;
    buf->num_rows = 0;
    buf->cursor_col = 0;
    buf->cursor_row = 0;
    buf->row_offset = 1; // Leave room for title bar.
    buf->col_offset = 0;
    buf->input_mode = IMODE_INSERT;

    buffer_insert_row(buf, 0);


    //print_buffer_linked_list(buf);
    //validate_linked_list(buf);
    return buf;
}

void buffer_free(Buffer* buf) {
    for(size_t i = 0; i < buf->num_row_nodes; i++) {
        bufrow_free(&buf->rows[i]);
    }

    buf->num_row_nodes = 0;
    free(buf->rows);

    free(buf);
}


void buffer_move_cursor_to(Buffer* buf, ssize_t row, ssize_t col) {
    Bufrow* bufrow = buffer_get_row(buf, row);
    if(bufrow == NULL) {
        return;
    }

    buf->cursor_row = row;
    buf->cursor_col = col;

    if(buf->cursor_row < 0) {
        buf->cursor_row = 0;
    }
    else
    if(buf->cursor_row >= buf->num_rows) {
        buf->cursor_row = buf->num_rows-1;
    }
    
    if(buf->cursor_col < 0) {
        buf->cursor_col = 0;
    }
    else
    if(buf->cursor_col > bufrow->len) {
        buf->cursor_col = bufrow->len;
    }
}


void buffer_move_cursor(Buffer* buf, int row_offset, int col_offset) {
    buffer_move_cursor_to(buf, 
            buf->cursor_row + row_offset,
            buf->cursor_col + col_offset);
}

bool buffer_insert_row(Buffer* buf, size_t position) {
    if(position > buf->num_row_nodes + 1) {
        return false;
    }


    if(buf->num_row_nodes_alloc <= position) {
        size_t new_num_row_nodes = (size_t)position + 64;
        buf->rows 
            = realloc
            (
                buf->rows,
                new_num_row_nodes * sizeof *buf->rows
            );
    
        for(size_t i = buf->num_row_nodes;
                   i < new_num_row_nodes; i++) {
            bufrow_allocate(&buf->rows[i]);
        }

        buf->num_row_nodes_alloc = new_num_row_nodes;
        logprintf(LOG_INFO, "Allocated some rows!");
    }

    if(buf->rows_head == NULL) {
        buf->rows_head = &buf->rows[0];
    }
    

    Bufrow* new_row = &buf->rows[buf->num_row_nodes++];
    Bufrow* row = buffer_get_row(buf, position);

    if(row == NULL) {
        return false;
    }

    if(new_row != row) {
    
        new_row->next = row->next;
        row->next = new_row;
       
        if(new_row->next) {
            new_row->next->prev = new_row; 
        }

        new_row->prev = row;
    }

    if(new_row->prev == NULL) {
        buf->rows_head = new_row;
    }

    buf->num_rows++;
    return true;
}

bool buffer_delete_row(Buffer* buf, size_t position) {

    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return false;
    }

    for(size_t i = 0; i < position; i++) {
        if(!row->next) { break; }
        row = row->next;
    }


    if(row->next) {
        row->next->prev = row->prev;
    }

    if(row->prev) {
        row->prev->next = row->next;
    }
    else {
        buf->rows_head = row->next;
    }


    row->next = NULL;
    row->prev = NULL;


    printf("Remove %p | TODO: Nodes may become fragmented.\n", row);
    buf->num_rows++;

    return true;
}

Bufrow* buffer_get_row(Buffer* buf, ssize_t position) {
    if(position < 0 || position > buf->num_rows + 1) {
        return NULL;
    }
    Bufrow* row = buf->rows_head;

    for(ssize_t i = 0; i < position; i++) {
        if(!row->next) { break; }
        row = row->next;
    }

    return row;
}



