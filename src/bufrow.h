#ifndef TXMODE_BUFROW_H
#define TXMODE_BUFROW_H

#include "types.h"


typedef struct Bufrow_t {
    char* data;
    size_t len;
    size_t memsize; // Allocated data size.

    struct Bufrow_t* prev;
    struct Bufrow_t* next;

    bool dirty; // If set to 'true', the whole row is cleared before writing to terminal.
}
Bufrow;



void bufrow_allocate   (Bufrow* row);
void bufrow_free       (Bufrow* row);

void bufrow_insert_char (Bufrow* row, ssize_t position, char c);
void bufrow_delete_char (Bufrow* row, ssize_t position);



#endif
