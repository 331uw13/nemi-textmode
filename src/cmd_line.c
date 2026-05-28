#include <stdio.h>
#include "text_mode.h"
#include "nemi.h"




void handle_cmd_line_chrpress(char c) {
    TXModest* txmst = get_txmst();
    string_addbyte(&txmst->cmd_str, c, txmst->cmd_line_cursor);
    txmst->cmd_line_cursor++;
}

void handle_cmd_line_keypress(int key, int mods) {
    (void)mods;
    
    TXModest* txmst = get_txmst();
    switch(key) {

        case GLFW_KEY_LEFT:
            if(txmst->cmd_line_cursor > 0) {
                txmst->cmd_line_cursor--;
            }
            break;

        case GLFW_KEY_RIGHT:
            if(txmst->cmd_line_cursor+1 <= (ssize_t)txmst->cmd_str.size) {
                txmst->cmd_line_cursor++;
            }
            break;

        case GLFW_KEY_BACKSPACE:
            if(txmst->cmd_line_cursor > 0) {
                string_delbyte(&txmst->cmd_str, txmst->cmd_line_cursor-1);
                txmst->cmd_line_cursor--;
            }
            break;

        case GLFW_KEY_ENTER:
            txmst->cmd_line_enabled = false;
            string_nullterm(&txmst->cmd_str);
            {

                if(strcmp(txmst->cmd_str.bytes, "close") == 0) {
                    delete_buffer(txmst->buffer);
                }

            }
            txmst->cmd_line_cursor = 0;
            string_clear(&txmst->cmd_str);
            break;
    }

}
