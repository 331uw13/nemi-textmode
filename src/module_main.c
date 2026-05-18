#include "nemi.h"



#include "text_mode.h"

#include <stdio.h>



static const int KEYBIND_TOGGLE[] = {
    GLFW_KEY_E,
    GLFW_KEY_LEFT_CONTROL
};


void module_event_render() {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    Buffer* buffer = txmst->buffer;
    if(buffer == NULL) {
        return;
    }


    // Cursor

    int cursor_x = nmt_coltox(nemi, 
            buffer->cursor_col + buffer->col_offset);

    int cursor_y = nmt_rowtoy(nemi, 
            buffer->cursor_row + buffer->row_offset);

    leaf_draw_rect
    (
        cursor_x,
        cursor_y,
        nemi->font.char_width,
        nemi->font.char_height,
        (struct color_t) {
            10, 60, 60
        }
    );


    // Buffer title.

    leaf_draw_rect
    (
        0,
        0,
        nemi->lfctx->win_width,
        nemi->font.char_height + 2,
        (struct color_t) {
            20, 20, 20
        }
    );

    char title_str[32] = { 0 };

    nemi->font.char_color_r = 0.3f;
    nemi->font.char_color_g = 0.3f;
    nemi->font.char_color_b = 0.3f;


    switch(buffer->input_mode) {
        case IMODE_INSERT:
            snprintf(title_str, sizeof(title_str)-1,
                    "mode:insert / rows:%li", buffer->num_rows);
            nemi->font.char_color_r = 0.7f;
            nemi->font.char_color_g = 0.36f;
            nemi->font.char_color_b = 0.3f;
            break;

        case IMODE_VIEW:
            snprintf(title_str, sizeof(title_str)-1,
                    "mode:view");
            nemi->font.char_color_r = 0.4f;
            nemi->font.char_color_g = 0.7f;
            nemi->font.char_color_b = 0.6f;
            break;

        default:
            snprintf(title_str, sizeof(title_str)-1,
                    "mode:unknown");
            break;
    }

    leaf_draw_text
    (
        &nemi->font,
        5,
        0,
        title_str, strlen(title_str)
    );
}


void module_event_key_input(int key, int mods) {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    if(!txmst->buffer) {
        return;
    }


    handle_input_mode_shared_keypress(key, mods);

    switch(txmst->buffer->input_mode) {
        case IMODE_INSERT:
            handle_input_mode_insert_keypress(key, mods);
            break;

        case IMODE_VIEW:
            handle_input_mode_view_keypress(key, mods);
            break;

        default:
            logprintf(LOG_ERROR, "(textmode.so): Input mode not handled. (key press)");
            break;
        // ...
    }
    
    update_text_to_terminal();
}

void module_event_char_input(char ch) {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    if(ch < 0x20 || ch > 0x7E) {
        return; // Dont accept non-ascii characters.
    }


    if(!txmst->buffer) {
        return;
    }

    switch(txmst->buffer->input_mode) {
        case IMODE_INSERT:
            handle_input_mode_insert_charpress(ch);
            break;

        case IMODE_VIEW:
            handle_input_mode_view_charpress(ch);
            break;

        default:
            logprintf(LOG_ERROR, "(textmode.so): Input mode not handled. (char press)");
            break;
        // ...
    }

    /*
    Bufrow* row = buffer_get_row(txmst->buffer, txmst->buffer->cursor_row);
    bufrow_insert_char(row, txmst->buffer->cursor_col, ch);
    txmst->buffer->cursor_col++;
    */

    update_text_to_terminal();
}


void toggle_module() {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();

    if(txmst->enabled) {
        txmst->enabled = false;
        nmt_module_free_inputfocus(nemi, txmst->module_idx);
        
        nmt_switch_terminal_ptr(nemi, nemi->terminal_prev);
    }
    else {
    
        if(!nmt_module_gain_inputfocus(nemi, txmst->module_idx)) {
            logprintf(LOG_ERROR, "(textmode.so) Failed to gain input focus.");
            return;
        }
        txmst->enabled = true;

        nmt_switch_terminal_ptr(nemi, txmst->term);
        nmterm_clear(txmst->term);
    }
}



void module_loaded(size_t module_idx) {
    create_txmst();
    TXModest* txmst = get_txmst();
    txmst->enabled = false;
    txmst->module_idx = module_idx;
    
    Nemi* nemi = nmt_getst();
    nmt_assign_module_keybind(nemi, module_idx, toggle_module, KEYBIND_TOGGLE, ARRAY_LEN(KEYBIND_TOGGLE));
    /*
    Nemi* nemi = nmt_getst();

    create_txmst();


    TXModest* txmst = get_txmst();


    txmst->module_idx = module_idx;
    txmst->enabled = true;

    if(!nmt_module_gain_inputfocus(nemi, module_idx)) {
        logprintf(LOG_ERROR, "TextMode failed to gain input focus.");
        return;
    }
    
    nmt_switch_terminal_ptr(nemi, txmst->term);
    nmterm_clear(txmst->term);
    */

    //nmterm_mv_putchr(txmst->term, 5, 5, 'A');

}


void module_quit() {
    free_txmst();
}


