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
    g_txmst->buffer_prev = NULL;
    g_txmst->clipboard = string_create(0);
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
    free_string(&g_txmst->clipboard);
    free(g_txmst);
}

TXModest* get_txmst() {
    return g_txmst;
}

static
bufuid_t p_get_buffer_uid() {
    bufuid_t uid = 0;

    bool is_unique = false;
    while(!is_unique) {

        uid = rand();

        // Assume the identifier is unique but confirm.
        is_unique = true;
        for(size_t i = 0; i < MAX_BUFFERS; i++) {
            if(g_txmst->buffers[i] == NULL) {
                continue;
            }

            if(g_txmst->buffers[i]->uid == uid) {
                is_unique = false;
                break;
            }
        }
    }

    return uid;
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
        buf->uid = p_get_buffer_uid();
        buf->name = strdup(name);
        buf->flags = flags;
        buf->input_mode = initial_imode;

        size_t buf_name_len = strlen(buf->name);
  
        buf->status_name = (BufferStatusName) {
            .name_ptr          = buf->name,
            .name_len          = buf_name_len,
            .name_len_original = buf_name_len,
            .scroll_timer_sec               = 0.0f,
            .scroll_timer_delay_restart_sec = 0.0f,
        };

        IModeCallbacks* imode_calls = &g_txmst->imode_callbacks[initial_imode];
        if(imode_calls->buffer_added) {
            imode_calls->buffer_added(buf);
        }

        switch_to_buffer(buf);
    }

    return buf;
}

void clean_all_buffers() {
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        if(g_txmst->buffers[i] == NULL) {
            continue;
        }

        buffer_clean_visible_rows(g_txmst->buffers[i]);
    }
   
    g_txmst->update_buffers = true;
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

    clean_all_buffers();

    if(was_active_buffer) {
        g_txmst->buffer = NULL;
        switch_to_buffer(g_txmst->buffer_prev);
    }
}

void switch_to_buffer(Buffer* buf) {
    if(buf == NULL) {
        return;
    }
    if(g_txmst->buffer) {
        buffer_clean_visible_rows(g_txmst->buffer); // Clean the current buffer first.
    }

    g_txmst->buffer_prev = g_txmst->buffer;
    g_txmst->buffer = buf;
    if(buf != NULL) {
        buffer_clean_visible_rows(buf);
    }
        
    g_txmst->update_buffers = true;
}

void update_buffers() { 
        
    buffer_write_to_terminal(g_txmst->buffer);
    g_txmst->update_buffers = false;
}

