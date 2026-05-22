#include "input_modes.h"
#include "../text_mode.h"

#include "nemi.h"

#include <stdio.h>


void handle_input_mode_insert_keypress(int key, int mods) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;
    
    Bufrow* row = buffer_get_row(buffer, buffer->cursor_row);
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


void handle_input_mode_insert_charpress(char c) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;

    Bufrow* row = buffer_get_row(buffer, buffer->cursor_row);
    if(row == NULL) {
        return;
    }

    bufrow_insert_char(row, buffer->cursor_col, c);
    buffer->cursor_col++;
}

