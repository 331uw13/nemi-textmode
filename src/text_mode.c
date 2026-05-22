#include "text_mode.h"
#include "nemi.h"

#include <stdio.h>

static TXModest* g_txmst = NULL;



void create_txmst() {
    g_txmst = malloc(sizeof *g_txmst);


    Nemi* nemi = nmt_getst();

    g_txmst->term 
        = nmterm_spawn
        (
            nemi, 
            nemi->win_rows,
            nemi->win_cols,
            ECHO_TERMINAL
        );

    memset(g_txmst->buffers, 0, sizeof *g_txmst->buffers * MAX_BUFFERS);


    g_txmst->files_buffer_idx = -1;

    
    add_new_buffer("test",
            (GridRect){ .col = 0, .row = 0, .max_row = BUFFER_FULL_MAX_ROW, .max_col = BUFFER_FULL_MAX_COL },
            BUFFER_NO_FLAGS);

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



Buffer* add_new_buffer(char* title, GridRect rect, uint64_t flags) {
    Buffer* buf = NULL;

    // Search space for new buffer to add.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            g_txmst->buffers[i] = buffer_allocate(rect.max_row, rect.max_col);
            buf = g_txmst->buffers[i];
            buf->index = i;
            buf->position_row = rect.row;
            buf->position_col = rect.col;
            buf->title = strdup(title);
            buf->flags = flags;
            break;
        }
    }

    return buf;
}

void delete_buffer(Buffer* buf) {
    buffer_clean_visible_rows(buf);

    size_t index = buf->index;
    buffer_free(buf);
    g_txmst->buffers[index] = NULL;

    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            continue;
        }

        buffer_clean_visible_rows(g_txmst->buffers[i]);
    }
}

void update_buffers() {
    
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* buf = g_txmst->buffers[i];
        if(buf == NULL) {
            continue;
        }

        buffer_write_to_terminal(buf);
    }

    g_txmst->update_buffers = false;
}

