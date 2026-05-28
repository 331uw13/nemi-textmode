#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"

#include <stdio.h>


void imode_INSERT_keypress(Buffer* buf, int key, int mods) {
    if(buf->flags & BUFFER_READONLY_FOR_USER) {
        return;
    }
    
    Bufrow* row = buffer_get_row(buf, buf->cursor_row);
    if(row == NULL) {
        return;
    }

    if(imode_basic_cursor_movement(buf, key, mods)) {
        return;
    }

    switch(key) {
        case GLFW_KEY_ENTER:
            buffer_eventkey_enter(buf, row);
            break;

        case GLFW_KEY_BACKSPACE:
            buffer_eventkey_backspace(buf, row);
            break;

        case GLFW_KEY_TAB:
            for(int i = 0; i < buf->settings.tab_width_in; i++) {
                bufrow_insert_char(row, buf->cursor_col, ' ');
            }
            buffer_move_cursor(buf, 0, buf->settings.tab_width_in);
            break;
    }
}


void imode_INSERT_chrpress(Buffer* buf, char c) {
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

