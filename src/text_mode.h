#ifndef NEMI_TEXT_MODE_H
#define NEMI_TEXT_MODE_H

#include "terminal.h"
#include "buffer.h"
#include "nmt_string.h"

#define MAX_BUFFERS 24


typedef struct TXModest_t {
    size_t module_idx;
    NTerminal* term;
    
    Buffer* buffers [MAX_BUFFERS];
    Buffer* buffer; // Current active buffer.
    Buffer* buffer_prev;

    bool enabled;
    bool update_buffers; // By setting this to 'true' will cause all buffers to be updated once.


    IModeCallbacks imode_callbacks [IMODE_COUNT];

    bool            cmd_line_enabled;
    ssize_t         cmd_line_cursor;
    struct string_t cmd_str;
}
TXModest;


void      create_txmst();
void      free_txmst();
TXModest* get_txmst();

Buffer*   add_new_buffer   (char* name, InputMode initial_imode, uint64_t flags);
void      delete_buffer    (Buffer* buf);
void      switch_to_buffer (Buffer* buf);
void      update_buffers();





#endif
