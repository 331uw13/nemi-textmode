#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"

#include <stdio.h>


void imode_INSERT_keypress(Buffer* buf, int key, int mods) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;
    

    if(buf->flags & BUFFER_READONLY_FOR_USER) {
        return;
    }

    Bufrow* row = buffer_get_row(buffer, buf->cursor_row);
    if(row == NULL) {
        return;
    }

    if(imode_basic_cursor_movement(buffer, key, mods)) {
        return;
    }

    switch(key) {
        case GLFW_KEY_ENTER:
            buffer_eventkey_enter(buffer, row);
            break;

        case GLFW_KEY_BACKSPACE:
            buffer_eventkey_backspace(buffer, row);
            break;

        case GLFW_KEY_TAB:
            for(int i = 0; i < buffer->settings.tab_width_in; i++) {
                bufrow_insert_char(row, buf->cursor_col, ' ');
            }
            buffer_move_cursor(buffer, 0, buf->settings.tab_width_in);
            break;
    }
}


void imode_INSERT_chrpress(Buffer* buf, char c) {
    TXModest* txmst = get_txmst();

    if(buf->flags & BUFFER_READONLY_FOR_USER) {
        return;
    }
    Bufrow* row = buffer_get_row(buf, buf->cursor_row);
    if(row == NULL) {
        return;
    }

    bufrow_insert_char(row, buf->cursor_col, c);
    buf->cursor_col++;
}

