#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bufrow.h"
#include "buffer.h"


#define MEMSIZE_INCREASE_AMOUNT 64


void bufrow_allocate(Bufrow* row, Buffer* parent) {
    row->data = calloc(64, sizeof *row->data);
    row->prev = NULL;
    row->next = NULL;
    row->len = 0;
    row->memsize = 0;
    row->dirty = true;
    row->parent_buffer = parent;
    row->number = 0;
}

void bufrow_free(Bufrow* row) {
    if(row->data) {
        free(row->data);
    }

    row->len = 0;
}

static
void bufrow_prep_memory(Bufrow* row, size_t new_length) {
    if(row->memsize <= new_length) {
        size_t new_size = new_length + MEMSIZE_INCREASE_AMOUNT; 
        row->memsize = new_size;
        row->data 
            = realloc
            (
                row->data,
                new_size
            );
    }
}

char* bufrow_datadup(Bufrow* row) {
    if(row->data == NULL || row->len == 0)  {
        return NULL;
    }

    char* d = malloc(row->len + 1);
    d[row->len] = '\0';

    return d;
}

char* bufrow_datadup_view (Bufrow* row, size_t column, size_t len) {
    if(row->data == NULL || row->len == 0 || len == 0 || column > row->len)  {
        return NULL;
    }

    size_t idx = 0;
    char* d = malloc(len + 1);
    
    for(size_t i = column; i < column + len; i++) {
        d[idx] = row->data[i];
        idx++;
    }
    d[len] = '\0';
    return d;
}


void bufrow_insert_char(Bufrow* row, ssize_t position, char c) {
    bufrow_prep_memory(row, row->len + 1);

    for(ssize_t i = row->len; i >= position && i > 0; i--) {
        row->data[i] = row->data[i-1];
    }

    row->data[position] = c;
    row->len++;
    row->dirty = true;


    undostack_push(&row->parent_buffer->undostack,
    (UndoStackCmd) {
        .kind = UCMD_INSERT_CHAR,
        .location = (UndoStackCmdLocation) {
            .buf_id = buffer_getid(row->parent_buffer),
            .row = row->number,
            .col = position
        }
    });
}

void bufrow_delete_char(Bufrow* row, ssize_t position) {
    if(row->data == NULL || row->len == 0 || position < 0) {
        return;
    }
    if(position >= (ssize_t)row->len) {
        row->len--;
        return;
    }

    const char deleted_char = row->data[position];

    for(ssize_t i = position; i < (ssize_t)row->len-1; i++) {
        row->data[i] = row->data[i+1];
    }

    row->len--;
    row->dirty = true;


    undostack_push(&row->parent_buffer->undostack,
    (UndoStackCmd) {
        .kind = UCMD_DELETE_CHAR,
        .location = (UndoStackCmdLocation) {
            .buf_id = buffer_getid(row->parent_buffer),
            .row = row->number,
            .col = position
        },
        .data_as.character = deleted_char,
        .data_len = 1
    });
}


void bufrow_cut(Bufrow* row, ssize_t position, size_t len) {
    if(position + len > row->len) {
        return;
    }


    if(!(row->parent_buffer->undostack.flags & UNDOSTACK_IGNORE_PUSH)) {
        undostack_push(&row->parent_buffer->undostack,
        (UndoStackCmd) {
            .kind = UCMD_DELETE_BLOCK,
            .location = (UndoStackCmdLocation) {
                .buf_id = buffer_getid(row->parent_buffer),
                .row = row->number,
                .col = position
            },
            .data_as.char_array = bufrow_datadup_view(row, position, len),
            .data_len = len
        });
    }

    size_t offset = 0;
    for(size_t i = position + len; i < row->len; i++) {
        row->data[position + offset] = row->data[i];
        offset++;
    }

    row->len -= len;

    /*
    // TODO: Set chars to zero and shift once by 'len'.
    for(size_t i = 0; i < len; i++) {
        bufrow_delete_char(row, position);
    }
    */
}

void bufrow_set(Bufrow* row, char* data, size_t len) {
    bufrow_prep_memory(row, len);
    memmove(row->data, data, len);
    row->len = len;
}

BufrowSubstr bufrow_substr (Bufrow* row, ssize_t position, size_t len) {
    BufrowSubstr substr = {
        .data_ptr = NULL,
        .len = 0
    };

    if(position + len > row->len) {
        goto out;
    }

    substr.data_ptr = row->data + position;
    substr.len = len;

out:
    return substr;
}

BufrowSubstr bufrow_substr_p(Bufrow* row, ssize_t begin, ssize_t end) {
    BufrowSubstr substr = {
        .data_ptr = NULL,
        .len = 0
    };

    if(begin > end) {
        goto out;
    }

    if(end > (ssize_t)row->len) {
        goto out;
    }

    substr.data_ptr = row->data + begin;
    substr.len      = end - begin;

out:
    return substr;
}

void bufrow_insert_substr(Bufrow* row, ssize_t position, BufrowSubstr substr) {
    if(position > (ssize_t)row->len || substr.data_ptr == NULL || substr.len == 0) {
        return;
    }

    for(size_t i = 0; i < substr.len; i++) {
        bufrow_insert_char(row, position + i, substr.data_ptr[i]);
    }
}

int bufrow_count_indent(Bufrow* row) {
    int count = 0;
    char* c = row->data;
    char* end = c + row->len;
    while(c < end) {
        if(*c != ' ') {
            break;
        }
        count++;
        c++;
    }
    return count;
}

