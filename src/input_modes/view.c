#include "input_modes.h"
#include "../text_mode.h"

#include "nemi.h"


void handle_input_mode_view_keypress(int key, int mods) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;


    switch(key) {

        case GLFW_KEY_I:
            if(mods & GLFW_MOD_CONTROL) {
                buffer->input_mode = IMODE_INSERT;
            }
            break;

    }

}


void handle_input_mode_view_charpress(char c) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;

    switch(c) {
  
        case 'i':
            buffer_move_cursor(buffer, -1, 0);
            break;

        case 'k':
            buffer_move_cursor(buffer, 1, 0);
            break;

        case 'j':
            buffer_move_cursor(buffer, 0, -1);
            break;
    
        case 'l':
            buffer_move_cursor(buffer, 0, 1);
            break;
    }


}

