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

    g_txmst->buffer = NULL;
    g_txmst->cmd_str = string_create(0);
    g_txmst->cmd_line_enabled = false;
    g_txmst->cmd_line_cursor = 0;
}


void free_txmst() {

    // Free buffers.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] != NULL) {
            buffer_free(g_txmst->buffers[i]);
        }
    }

    free_string(&g_txmst->cmd_str);
    free(g_txmst);
}

TXModest* get_txmst() {
    return g_txmst;
}



Buffer* add_new_buffer(char* name, InputMode initial_imode, uint64_t flags) {
    Buffer* buf = NULL;

    // Search space for new buffer to add.
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            g_txmst->buffers[i] = buffer_init();
            buf = g_txmst->buffers[i];
            buf->index = i;
            break;
        }
    }


    if(buf != NULL) {
        buf->name = strdup(name);
        buf->flags = flags;
        buf->input_mode = initial_imode;
   

        IModeCallbacks* imode_calls = &g_txmst->imode_callbacks[initial_imode];
        if(imode_calls->buffer_added) {
            imode_calls->buffer_added(buf);
        }

        switch_to_buffer(buf);
    }

    return buf;
}

void delete_buffer(Buffer* buf) {
    bool was_active_buffer = buf == g_txmst->buffer;

    IModeCallbacks* imode_calls = &g_txmst->imode_callbacks[buf->input_mode];
    if(imode_calls->buffer_free) {
        imode_calls->buffer_free(buf);
    }

    buffer_clean_visible_rows(buf);

    size_t index = buf->index;
    buffer_free(buf);
    g_txmst->buffers[index] = NULL;

    if(was_active_buffer) {
        g_txmst->buffer = NULL;
    }

    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            continue;
        }

        buffer_clean_visible_rows(g_txmst->buffers[i]);
    }
}

void switch_to_buffer(Buffer* buf) {
    if(g_txmst->buffer) {
        buffer_clean_visible_rows(g_txmst->buffer); // Clean the current buffer first.
    }

    g_txmst->buffer = buf;
    if(buf != NULL) {
        buffer_clean_visible_rows(buf);
    }
}

void update_buffers() { 
        
    buffer_write_to_terminal(g_txmst->buffer);
    g_txmst->update_buffers = false;
}

