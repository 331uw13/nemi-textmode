#include "bufrow.h"

#include <stdio.h>
#include <stdlib.h>


#define MEMSIZE_INCREASE_AMOUNT 64


void bufrow_allocate(Bufrow* row) {
    row->data = calloc(64, sizeof *row->data);
    row->prev = NULL;
    row->next = NULL;
    row->len = 0;
    row->memsize = 0;
    row->dirty = false;
}

void bufrow_free(Bufrow* row) {
    if(row->data) {
        free(row->data);
    }

    row->len = 0;
}



static
void bufrow_memprep_add(Bufrow* row, size_t num_bytes_adding) {
    if(row->memsize <= row->len + num_bytes_adding) {    
        size_t new_size = row->len + num_bytes_adding + MEMSIZE_INCREASE_AMOUNT; 

        row->data 
            = realloc
            (
                row->data,
                new_size
            );
    }
}

void bufrow_insert_char(Bufrow* row, ssize_t position, char c) {
    bufrow_memprep_add(row, 1);

    for(ssize_t i = row->len; i >= position && i > 0; i--) {
        row->data[i] = row->data[i-1];
    }

    row->data[position] = c;
    row->len++;
    row->dirty = true;
}

void bufrow_delete_char(Bufrow* row, ssize_t position) {
    if(row->len == 0 || position < 0) {
        return;
    }
    if(position >= (ssize_t)row->len) {
        row->len--;
        return;
    }

    for(ssize_t i = position; i < (ssize_t)row->len-1; i++) {
        row->data[i] = row->data[i+1];
    }

    row->len--;
    row->dirty = true;
}






