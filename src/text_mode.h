#ifndef NEMI_TEXT_MODE_H
#define NEMI_TEXT_MODE_H

#include "terminal.h"
#include "buffer.h"


#define MAX_BUFFERS 24


typedef struct TXModest_t {
    size_t module_idx;
    NTerminal* term;
    
    Buffer* buffers [MAX_BUFFERS];
    Buffer* buffer; // Current active buffer.

    bool enabled;
    bool update_buffers; // By setting this to 'true' will cause all buffers to be updated once.

    ssize_t files_buffer_idx;


    IModeCallbacks imode_callbacks [IMODE_COUNT];
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
