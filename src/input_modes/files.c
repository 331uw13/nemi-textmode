#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"

#include <stdio.h>



static
void p_list_files(Buffer* buf) {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    NModule* this_module = &nemi->modules[txmst->module_idx];
    
    Bufrow* row = buffer_insert_row(buf, buf->num_rows);
    bufrow_set(row, "Hello!", 6);



}

void imode_FILES_buffer_added(Buffer* buf) {

    p_list_files(buf);
}

void imode_FILES_keypress(Buffer* buf, int key, int mods) {
    TXModest* txmst = get_txmst();


    switch(key) {
        case GLFW_KEY_I:
            buffer_move_cursor(buf, -1, 0);
            break;
        
        case GLFW_KEY_K:
            buffer_move_cursor(buf, 1, 0);
            break;
    }

}

void imode_FILES_chrpress(Buffer* buf, char c) {
    TXModest* txmst = get_txmst();


}
