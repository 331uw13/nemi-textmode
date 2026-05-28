#ifndef TXMODE_BUFFER_H
#define TXMODE_BUFFER_H

#include "types.h"
#include "bufrow.h"
#include "select_region.h"
#include "input_modes.h"


#define BUFFER_NO_FLAGS 0
#define BUFFER_NO_NUMBER         (1 << 0) // No row numbers.
#define BUFFER_NO_MODE_IN_TITLE  (1 << 1) // No mode string in title bar.
#define BUFFER_READONLY_FOR_USER (1 << 2) // The user cannot edit text, but functions which can, still work.
#define BUFFER_IMODE_CANT_CHANGE (1 << 3) // Buffer input mode cant be changed with user keypress event.


typedef int bufuid_t;


typedef struct BufferStatusName_t {
    char*    name_ptr;
    ssize_t  name_len;
    ssize_t  name_len_original;
    float    scroll_timer_sec;
    float    scroll_timer_delay_restart_sec;
}
BufferStatusName;


typedef struct Buffer_t {
    size_t   index;
    uint64_t flags;
    bufuid_t uid; // Used for undo stack operations to identify the buffer.
    BufferStatusName status_name;

    char*    name;
    char*    opened_file_path;
    Bufrow*  rows_head;
    Bufrow*  rows_tail;
    Bufrow*  rows;
    //size_t   num_row_nodes;
    //size_t  num_row_nodes_alloc;
    
    size_t  num_rows;

    ssize_t cursor_row;
    ssize_t cursor_col;
    
    ssize_t   yscroll; // Vertical scroll.

    int     row_offset; // How many rows to offset drawing.
    int     col_offset; // How many columns to offset drawing.

    int     max_row;
    int     max_col;
    
    NSelectRegion  select;

    struct {
        int tab_width_in;  // Tab width in editor.
        int tab_width_out; // Tab width when writing to a file.
    }
    settings;

    InputMode     input_mode;
    void*         user_pointer; // Mainly reserved for custom input modes.
}
Buffer;

typedef struct BufferID_t {
    bufuid_t buffer_uid;
    size_t   buffer_index;
}
BufferID;

inline BufferID buffer_getid(Buffer* buf) {
    return (BufferID) {
        .buffer_uid = buf->uid,
        .buffer_index = buf->index
    };
}




Buffer* buffer_init ();
void    buffer_free (Buffer* buf);


void buffer_move_cursor_to     (Buffer* buf, ssize_t row, ssize_t col); 
void buffer_move_cursor        (Buffer* buf, int row_offset, int col_offset);

Bufrow* buffer_insert_row      (Buffer* buf, size_t position);
bool    buffer_delete_row      (Buffer* buf, size_t position);
void    buffer_delete_all_rows (Buffer* buf);
Bufrow* buffer_get_row         (Buffer* buf, ssize_t position);

int buffer_screen_max_row      (Buffer* buf);
int buffer_screen_max_col      (Buffer* buf);

void buffer_eventkey_enter     (Buffer* buf, Bufrow* row);
void buffer_eventkey_backspace (Buffer* buf, Bufrow* row);

void buffer_yscroll_to         (Buffer* buf, ssize_t position);
void buffer_yscroll            (Buffer* buf, int offset);

void buffer_clean_visible_rows (Buffer* buf);
void buffer_write_to_terminal  (Buffer* buf);

void buffer_read_file          (Buffer* buf, const char* file_path);
void buffer_save_file          (Buffer* buf);
/*
void buffer_insert_char (Buffer* buf, ssize_t row, ssize_t column, char c);
void buffer_delete_char (Buffer* buf, ssize_t row, ssize_t column);
*/


void buffer_select_render_callback(void* userptr, ssize_t row, ssize_t row_length, ssize_t col_begin);



#endif
