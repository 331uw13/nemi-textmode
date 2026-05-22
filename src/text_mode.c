#include "text_mode.h"
#include "nemi.h"

#include <stdio.h>

static TXModest* g_txmst = NULL;



void create_txmst() {
    g_txmst = malloc(sizeof *g_txmst);


    Nemi* nemi = nmt_getst();

    g_txmst->term =
        nmterm_spawn
        (
            nemi, 
            nemi->win_rows,
            nemi->win_cols,
            ECHO_TERMINAL
        );

    memset(g_txmst->buffers, 0, sizeof *g_txmst->buffers * MAX_BUFFERS);


    add_new_buffer("test", 0, 0, BUFFER_FULL_MAX_ROW, BUFFER_FULL_MAX_COL, BUFFER_INTERACTABLE);
    

    Buffer* files_buf = add_new_buffer("another buffer", 5, 5, 10, 30, BUFFER_INTERACTABLE);

    g_txmst->buffer = g_txmst->buffers[0];
}


void free_txmst() {

    // Free buffers.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] != NULL) {
            buffer_free(g_txmst->buffers[i]);
        }
    }
    free(g_txmst);
}

TXModest* get_txmst() {
    return g_txmst;
}



Buffer* add_new_buffer(char* title, int row, int col, int max_row, int max_col, uint64_t flags) {
    Buffer* buf = NULL;

    // Search space for new buffer to add.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            g_txmst->buffers[i] = buffer_allocate(max_row, max_col);
            buf = g_txmst->buffers[i];
            buf->position_row = row;
            buf->position_col = col;
            buf->title = strdup(title);
            buf->flags = flags;
            break;
        }
    }

    return buf;
}



static
void p_update_buffer(Buffer* buf) {
    if(buf->flags & BUFFER_HIDE) {
        return;
    }

    ssize_t row_counter = 0;
    Bufrow* row = buffer_get_row(buf, buf->yscroll);
   


    char linenum_buf[32] = { 0 };

    if(!(buf->flags & BUFFER_NONUMBER)) {
        buf->col_offset = snprintf(linenum_buf, sizeof(linenum_buf)-1,
                "%li", buf->num_rows) + 1;
    }

    //nmterm_clear_row_part(g_txmst->term, );
    //nmterm_clear_row(g_txmst->term, buf->position_col + buf->num_rows+1 - buf->yscroll);
    nmterm_clear_row_part(g_txmst->term, 
            buf->position_row + buf->num_rows+1 - buf->yscroll,
            buf->position_col, buf->position_col + buf->max_col);


    while(row) {

        /*
        if(buf->flags & BUFFER_CLEAN_VISIBLE_ROWS) {
            row->dirty = true;
        }
        */

        if(row->dirty) {
            int real_row = buf->position_row + buf->row_offset + row_counter;
            
            nmterm_clear_row_part(g_txmst->term, real_row, buf->position_col, buf->position_col + buf->max_col);


            if(!(buf->flags & BUFFER_NONUMBER)) {
                ssize_t row_number = row_counter + buf->yscroll;
                ssize_t linenum_buf_len 
                    = snprintf(linenum_buf, sizeof(linenum_buf)-1, "\033[90m%li\033[0m", row_number);

                nmterm_mv_putstrn
                (
                    g_txmst->term,
                    real_row,
                    buf->position_col,
                    linenum_buf,
                    linenum_buf_len
                );
            }

            for(size_t col = 0; col < row->len; col++) {

                nmterm_mv_putchr
                (
                    g_txmst->term,
                    real_row,
                    buf->position_col + buf->col_offset + col,
                    row->data[col]
                );
            }
        }

        row = row->next;
        row_counter++;

        if(row_counter >= buffer_screen_max_row(buf)) {
            break;
        }
    }
        
    //buf->flags &= ~BUFFER_CLEAN_VISIBLE_ROWS;
}

void update_text_to_terminal() {
    
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* buf = g_txmst->buffers[i];
        if(buf == NULL) {
            continue;
        }

        p_update_buffer(buf);
    }
}

