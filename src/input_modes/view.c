#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"



void imode_VIEW_keypress(Buffer* buf, int key, int mods) {
    imode_basic_cursor_movement(buf, key, mods);
}


void imode_VIEW_chrpress(Buffer* buf, char c) {

    (void)buf;
    (void)c;

}

