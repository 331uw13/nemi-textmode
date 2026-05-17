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

    int row_counter = 0;
    Bufrow* row = buffer_get_row(buf, 0);
    while(row) {
        if(row->dirty) {
            int real_row = buf->row_offset + row_counter;

            nmterm_clear_row(g_txmst->term, real_row);

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
    }
}

void update_text_to_terminal() {
    // This will be changed later to support multiple buffers.
    p_update_buffer_to_terminal(g_txmst->buffer);
}

