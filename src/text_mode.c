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
    add_new_buffer();

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

Buffer* add_new_buffer() {
    Buffer* buf = NULL;

    // Search space for new buffer to add.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            g_txmst->buffers[i] = buffer_allocate();
            buf = g_txmst->buffers[i];
            break;
        }
    }

    return buf;
}

static
void p_update_buffer_to_terminal(Buffer* buf) {

    ssize_t row_counter = 0;
    Bufrow* row = buffer_get_row(buf, buf->yscroll);
   


    char linenum_buf[32] = { 0 };

    buf->col_offset = snprintf(linenum_buf, sizeof(linenum_buf)-1,
            "%li", buf->num_rows) + 1;


    nmterm_clear_row(g_txmst->term, buf->num_rows+1);
    while(row) {
        if(row->dirty) {
            int real_row = buf->row_offset + row_counter;
            nmterm_clear_row(g_txmst->term, real_row);

            ssize_t linenum_buf_len 
                = snprintf(linenum_buf, sizeof(linenum_buf)-1, "\033[90m%li\033[0m", row_counter);
            nmterm_mv_putstrn
            (
                g_txmst->term,
                real_row,
                0,
                linenum_buf,
                linenum_buf_len
            );

            for(size_t col = 0; col < row->len; col++) {

                nmterm_mv_putchr
                (
                    g_txmst->term,
                    real_row,
                    buf->col_offset + col,
                    row->data[col]
                );

            }
        }

        row = row->next;
        row_counter++;

        if(row_counter >= buffer_real_max_row(buf)) {
            break;
        }
    }
}

void update_text_to_terminal() {
    // This will be changed later to support multiple buffers.
    p_update_buffer_to_terminal(g_txmst->buffer);
}

