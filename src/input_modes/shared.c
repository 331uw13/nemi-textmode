#include "input_modes.h"
#include "../text_mode.h"

#include "nemi.h"


void handle_input_mode_shared_keypress(int key, int mods) {
    TXModest* txmst = get_txmst();
    Buffer* buffer = txmst->buffer;

    switch(key) {

        case GLFW_KEY_ESCAPE:
            buffer->input_mode = IMODE_VIEW;
            break;

        case GLFW_KEY_X:
            if(mods & GLFW_MOD_CONTROL) {
                buffer->input_mode = IMODE_VIEW;
            }
            break;

    }

}
