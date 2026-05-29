#ifndef TXMODE_BUFROW_H
#define TXMODE_BUFROW_H

#include "types.h"


typedef struct Buffer_t Buffer;

typedef struct Bufrow_t {
    char* data;
    size_t len;
    size_t memsize; // Allocated data size.

    struct Bufrow_t* prev;
    struct Bufrow_t* next;
    Buffer* parent_buffer;

    size_t number; // The actual row number.
    bool dirty; // If set to 'true', the whole row is cleared before writing to terminal.
}
Bufrow;

typedef struct BufrowSubstr_t {
    char* data_ptr;
    size_t len;
}
BufrowSubstr;


void bufrow_allocate   (Bufrow* row, Buffer* parent);
void bufrow_free       (Bufrow* row);

void bufrow_insert_char   (Bufrow* row, ssize_t position, char c);
void bufrow_delete_char   (Bufrow* row, ssize_t position);
void bufrow_cut           (Bufrow* row, ssize_t position, size_t len);
void bufrow_set           (Bufrow* row, char* data, size_t len);
BufrowSubstr bufrow_substr        (Bufrow* row, ssize_t position, size_t len);
BufrowSubstr bufrow_substr_p      (Bufrow* row, ssize_t begin, ssize_t end);
void         bufrow_insert_substr (Bufrow* row, ssize_t position, BufrowSubstr substr);

// Returns the number of whitespace characters at the begin or data.
int          bufrow_count_indent  (Bufrow* row);


#endif
