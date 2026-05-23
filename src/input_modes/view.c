#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"


void imode_VIEW_keypress(Buffer* buf, int key, int mods) {
    TXModest* txmst = get_txmst();


}


void imode_VIEW_chrpress(Buffer* buf, char c) {
    TXModest* txmst = get_txmst();

    switch(c) {
  
        case 'i':
            buffer_move_cursor(buf, -1, 0);
            break;

        case 'k':
            buffer_move_cursor(buf, 1, 0);
            break;

        case 'j':
            buffer_move_cursor(buf, 0, -1);
            break;
    
        case 'l':
            buffer_move_cursor(buf, 0, 1);
            break;
    }


}

