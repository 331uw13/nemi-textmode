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

    switch(key) {

        case GLFW_KEY_I:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buffer, -1, 0);
            }
            break;

        case GLFW_KEY_K:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buffer, 1, 0);
            }
            break;
        case GLFW_KEY_J:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buffer, 0, -1);
            }
            break;

        case GLFW_KEY_L:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buffer, 0, 1);
            }
            break;

        case GLFW_KEY_ENTER:
            buffer_eventkey_enter(buffer, row);
            break;

        case GLFW_KEY_BACKSPACE:
            buffer_eventkey_backspace(buffer, row);
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

