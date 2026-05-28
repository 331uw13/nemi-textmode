#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"


void imode_SELECT_keypress(Buffer* buf, int key, int mods) {
    if(imode_basic_cursor_movement(buf, key, mods)) {
        nmt_select_move(&buf->select, buf->cursor_col, buf->cursor_row);
        return;
    }

    switch(key) {
        case GLFW_KEY_N:
            buf->select.mode = SREG_MODE_NORMAL;
            break;
        
        case GLFW_KEY_B:
            buf->select.mode = SREG_MODE_BLOCK;
            break;
        
        case GLFW_KEY_V:
            buf->select.mode = SREG_MODE_LINE;
            break;
    }
}


void imode_SELECT_chrpress(Buffer* buf, char c) {

    (void)buf;
    (void)c;

}
