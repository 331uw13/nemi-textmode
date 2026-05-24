#include "input_modes.h"
#include "nemi.h"
#include "buffer.h"


bool imode_basic_cursor_movement(Buffer* buf, int key, int mods) {

    switch(key) {

        case GLFW_KEY_I:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buf, -1, 0);
            }
            return true;

        case GLFW_KEY_K:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buf, 1, 0);
            }
            return true;
        
        case GLFW_KEY_J:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buf, 0, -1);
            }
            return true;

        case GLFW_KEY_L:
            if(mods & GLFW_MOD_CONTROL) {
                buffer_move_cursor(buf, 0, 1);
            }
            return true;

        case GLFW_KEY_UP:
            buffer_move_cursor(buf, -1, 0);
            return true;
        
        case GLFW_KEY_DOWN:
            buffer_move_cursor(buf, 1, 0);
            return true;
        
        case GLFW_KEY_LEFT:
            buffer_move_cursor(buf, 0, -1);
            return true;
        
        case GLFW_KEY_RIGHT:
            buffer_move_cursor(buf, 0, 1);
            return true;

    }

    return false;
}

