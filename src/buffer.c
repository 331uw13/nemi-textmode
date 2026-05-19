#include <assert.h>

#include "buffer.h"
#include "text_mode.h"

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

*/

static // Temporary function.
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
            for(size_t l = 0; l < row->len; l++) {
                printf("%c", row->data[l]);
            }
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



Buffer* buffer_allocate() {
    Buffer* buf = malloc(sizeof *buf);
    buf->rows = malloc(sizeof *buf->rows);
    bufrow_allocate(&buf->rows[0]);
    buf->rows_head = &buf->rows[0];
    buf->rows_tail = &buf->rows[0];

    //buf->num_row_nodes = 0;
    //buf->num_row_nodes_alloc = 0;
    buf->num_rows = 1;
    buf->cursor_col = 0;
    buf->cursor_row = 0;
    buf->row_offset = 1; // Leave room for title bar.
    buf->col_offset = 0;
    buf->yscroll = 0;
    buf->input_mode = IMODE_INSERT;
   
    TXModest* txmst = get_txmst();
    buf->max_row = txmst->term->rows;
    buf->max_col = txmst->term->cols;

    //buffer_insert_row(buf, 0);


    //print_buffer_linked_list(buf);
    //validate_linked_list(buf);
    return buf;
}

void buffer_free(Buffer* buf) {
    Bufrow* row = buf->rows_tail;
    while(row) {
        Bufrow* prev = row->prev;
        
        bufrow_free(row);
        if(row->next) {
            free(row->next);
        }

        row = prev;
    }

    free(buf->rows);
    free(buf);
}

int buffer_real_max_row(Buffer* buf) {
    return buf->max_row - buf->row_offset;
}

int buffer_real_max_col(Buffer* buf) {
    return buf->max_col - buf->col_offset;
}

void buffer_move_cursor_to(Buffer* buf, ssize_t row, ssize_t col) {
    Bufrow* bufrow = buffer_get_row(buf, row);
    if(bufrow == NULL) {
        return;
    }
    
    bool moved_up   = buf->cursor_row > row;
    bool moved_down = buf->cursor_row < row;


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


    if(moved_down) {
        if(buf->cursor_row - buf->yscroll >= buffer_real_max_row(buf)) {
            buffer_yscroll_to(buf, buf->cursor_row - buffer_real_max_row(buf) + 1);
        }
    }
    else
    if(moved_up) {
        if(buf->cursor_row < buf->yscroll) {
            buffer_yscroll_to(buf, buf->cursor_row);
        }
    }
    /*
    if(buf->cursor_row >= buffer_real_max_row(buf)) {
        buffer_yscroll(buf, 1);
    }
    */
    
    /*else
    if(buf->cursor_row <= buf->yscroll) {
        buffer_yscroll(buf, -1);
    }
    */
}

void buffer_move_cursor(Buffer* buf, int row_offset, int col_offset) {
    buffer_move_cursor_to(buf, 
            buf->cursor_row + row_offset,
            buf->cursor_col + col_offset);
}

void buffer_yscroll_to(Buffer* buf, ssize_t position) {
    if(position < 0) {
        return;
    }

    buf->yscroll = position;
}

void buffer_yscroll(Buffer* buf, int offset) {
    buffer_yscroll_to(buf, buf->yscroll + offset);
}

bool buffer_insert_row(Buffer* buf, size_t position) {
    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return false;
    }

    Bufrow* new_row = malloc(sizeof(Bufrow));
    bufrow_allocate(new_row);

    new_row->prev = row;
    new_row->next = row->next;

    if(row->next) {
        row->next->prev = new_row;
    }

    row->next = new_row;


    bool at_tail = row == buf->rows_tail;
    if(at_tail) {
        buf->rows_tail = new_row;
    }

    buf->num_rows++;
    //validate_linked_list(buf);

    buf->num_row_nodes++;
    return true;
}

bool buffer_delete_row(Buffer* buf, size_t position) {
    if(buf->num_rows == 1) {
        return false;
    }
    
   // validate_linked_list(buf);

    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return false;
    }

    if(row->next) {
        row->next->prev = row->prev;
    }
    else {
        buf->rows_tail = row;
    }

    if(row->prev) {
        row->prev->next = row->next;
    }
    else {
        buf->rows_head = row->next;
    }

    
    // Set below rows to re-written to terminal.
    Bufrow* row_iter = row;
    while(row_iter) {
        row_iter->dirty = true;
        row_iter = row_iter->next;
    }

    row->next = NULL;
    row->prev = NULL;




    //validate_linked_list(buf);

    printf("Remove %p  \033[33mTODO: Nodes may become fragmented.\033[0m\n", row);
    buf->num_rows--;

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


void buffer_eventkey_enter(Buffer* buf, Bufrow* row) {
    if(!buffer_insert_row(buf, buf->cursor_row)) {
        logprintf(LOG_ERROR, "(textmode.so) Failed to insert row (enter)");
        return;
    }

    // This is the new inserted row.
    Bufrow* row_below = row->next; 

    if(buf->cursor_col < row->len) {
        BufrowSubstr substr = bufrow_substr_p(row, buf->cursor_col, row->len);
    
        bufrow_insert_substr(row_below, 0, substr);
        bufrow_cut(row, buf->cursor_col, row->len - buf->cursor_col);
    }
        
    buffer_move_cursor_to(buf, buf->cursor_row+1, 0);
}

void buffer_eventkey_backspace(Buffer* buf, Bufrow* row) {
    if(buf->cursor_col > 0) {
        bufrow_delete_char(row, buf->cursor_col-1);
        buffer_move_cursor(buf, 0, -1);
        return;
    }

    if(row->len == 0) {
        buffer_delete_row(buf, buf->cursor_row);
        buffer_move_cursor(buf, -1, 0);
    }
    else
    if(row->prev) {

        size_t above_row_old_len = row->prev->len;
        BufrowSubstr substr = bufrow_substr_p(row, 0, row->len);
        bufrow_insert_substr(row->prev, row->prev->len, substr);

        buffer_delete_row(buf, buf->cursor_row);
        buffer_move_cursor_to(buf, buf->cursor_row-1, above_row_old_len);
    }
}

