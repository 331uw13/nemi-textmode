#ifndef TXMODE_BUFFER_H
#define TXMODE_BUFFER_H

#include "types.h"
#include "bufrow.h"
#include "input_modes/input_modes.h"



typedef struct Buffer_t {
   
    Bufrow* rows_head;
    Bufrow* rows_tail;
    Bufrow* rows;
    size_t  num_row_nodes;
    //size_t  num_row_nodes_alloc;
    
    size_t  num_rows;

    ssize_t cursor_row;
    ssize_t cursor_col;
    
    ssize_t   yscroll;

    int     row_offset;
    int     col_offset;

    int     max_row;
    int     max_col;


    InputMode input_mode;
}
Buffer;


Buffer* buffer_allocate();
void    buffer_free    (Buffer* buf);


void buffer_move_cursor_to (Buffer* buf, ssize_t row, ssize_t col); 
void buffer_move_cursor    (Buffer* buf, int row_offset, int col_offset);
bool buffer_insert_row     (Buffer* buf, size_t position);
bool buffer_delete_row     (Buffer* buf, size_t position);

Bufrow* buffer_get_row  (Buffer* buf, ssize_t position);
int buffer_real_max_row (Buffer* buf);
int buffer_real_max_col (Buffer* buf);

void buffer_eventkey_enter     (Buffer* buf, Bufrow* row);
void buffer_eventkey_backspace (Buffer* buf, Bufrow* row);

void buffer_yscroll_to  (Buffer* buf, ssize_t position);
void buffer_yscroll     (Buffer* buf, int offset);

/*
void buffer_insert_char (Buffer* buf, ssize_t row, ssize_t column, char c);
void buffer_delete_char (Buffer* buf, ssize_t row, ssize_t column);
*/



#endif
