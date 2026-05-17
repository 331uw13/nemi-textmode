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
}
TXModest;


void      create_txmst();
void      free_txmst();
TXModest* get_txmst();

Buffer*   add_new_buffer();
void      update_text_to_terminal();



#endif
