#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"



void imode_VIEW_keypress(Buffer* buf, int key, int mods) {
    if(imode_basic_cursor_movement(buf, key, mods)) {
        return;
    }

    TXModest* txmst = get_txmst();


    switch(key) {

        case GLFW_KEY_D:
            {
                Bufrow* row = buffer_get_row(buf, buf->cursor_row);
                if(row != NULL) {
                    string_move(&txmst->clipboard, row->data, row->len);
                    buffer_delete_row(buf, buf->cursor_row);
                }
            }
            break;

        case GLFW_KEY_P:
            {
                if(txmst->clipboard.size == 0) {
                    return;
                }

                Bufrow* row = buffer_insert_row(buf, buf->cursor_row);
                bufrow_set(row, txmst->clipboard.bytes, txmst->clipboard.size);
            }
            break;
    }
}


void imode_VIEW_chrpress(Buffer* buf, char c) {

    (void)buf;
    (void)c;

}

