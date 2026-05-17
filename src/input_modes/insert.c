#include "input_modes.h"
#include "../text_mode.h"

#include "nemi.h"


void handle_input_mode_insert_keypress(int key, int mods) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;
    
    Bufrow* row = buffer_get_row(buffer, buffer->cursor_row);
    if(row == NULL) {
        return;
    }

    switch(key) {

        case GLFW_KEY_BACKSPACE:
            bufrow_delete_char(row, buffer->cursor_col-1);
            buffer_move_cursor(buffer, 0, -1);
            break;

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
            {
                if(!buffer_insert_row(buffer, buffer->cursor_row)) {
                    logprintf(LOG_ERROR, "(textmode.so) Failed to insert row (enter)");
                    return;
                }

                Bufrow* row_below = row->next;

                if(buffer->cursor_col >= row->len) {
                    buffer_move_cursor_to(buffer, buffer->cursor_row+1, 0);
                }



            }
            break;
    }

}

#include <stdio.h>

void handle_input_mode_insert_charpress(char c) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;

    Bufrow* row = buffer_get_row(buffer, buffer->cursor_row);
    if(row == NULL) {
        printf("%s: This fucked up.\n",__func__);
        return;
    }

    bufrow_insert_char(row, buffer->cursor_col, c);
    buffer->cursor_col++;
}

