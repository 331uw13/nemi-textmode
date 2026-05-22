#include "nemi.h"



#include "text_mode.h"

#include <stdio.h>
#include <math.h>


static const int KEYBIND_TOGGLE[] = {
    GLFW_KEY_E,
    GLFW_KEY_LEFT_CONTROL
};




static
bool p_mouse_on_box(Nemi* nemi, Buffer* buf, int row, int col, int width_cols, int height_rows) {
    
    int box_begin_x = nmt_coltox(nemi, col);
    int box_begin_y = nmt_rowtoy(nemi, row);

    int box_end_x = nmt_coltox(nemi, col + width_cols);
    int box_end_y = nmt_rowtoy(nemi, row + height_rows);


    if(nemi->mouse_x >= box_begin_x && nemi->mouse_x < box_end_x
    && nemi->mouse_y >= box_begin_y && nemi->mouse_y < box_end_y) {
        return true;
    }

    return false;
}


static
void p_update_buffer_uiinteraction(Buffer* buf) {
    Nemi* nemi = nmt_getst();


    int mouse_col = floor(nemi->mouse_x / nemi->font.char_width);
    int mouse_row = floor(nemi->mouse_y / (nemi->font.char_height + nemi->cfg.main.line_padding));
    
    bool mouse_down = glfwGetMouseButton(nemi->lfctx->glfw_win, GLFW_MOUSE_BUTTON_LEFT);

    bool mouse_on_titlebar 
        = p_mouse_on_box(nemi, buf, buf->position_row, buf->position_col, buf->max_col, 1);


    // For how this will clear the buffer
    // data and keep it from updating it again.
    // Dont want to deal with keeping track of which column and row must be updated
    // while moving the buffer. TODO: ^ future improvement.

    if(mouse_on_titlebar && mouse_down) {
        if(!(buf->flags & BUFFER_MOUSE_DRAG_MOVE)) {
            // Drag was started so clear the rows.
            buffer_clean_visible_rows(buf);
        }
        buf->flags |= BUFFER_MOUSE_DRAG_MOVE;
        buf->flags |= BUFFER_HIDE;
    }
    else
    if(!mouse_down) {
        
        if(buf->flags & BUFFER_HIDE) {
            buf->flags &= ~BUFFER_HIDE;
            update_text_to_terminal();
        }
        buf->flags &= ~BUFFER_MOUSE_DRAG_MOVE;

    }


    if(buf->flags & BUFFER_MOUSE_DRAG_MOVE) {
        
        buf->position_col = mouse_col - buf->max_col / 2;   
        buf->position_row = mouse_row;
   

    }
    

    /*
    if(p_mouse_on_box(nemi, buf, buf->position_row, buf->position_col, buf->max_col, buf->max_row)) {
        printf("True\n");
    }
    else {
        printf("False\n");
    }
    */

}

void module_event_render() {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }




    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* buffer = txmst->buffers[i];
        if(buffer == NULL) {
            continue;
        }

        if(buffer->flags & BUFFER_INTERACTABLE) {
            p_update_buffer_uiinteraction(buffer);
        }

 
        if(!(buffer->flags & BUFFER_HIDE)) {

            // Cursor
            int cursor_x = nmt_coltox(nemi, 
                    buffer->position_col + buffer->cursor_col + buffer->col_offset);

            int cursor_y = nmt_rowtoy(nemi, 
                    buffer->position_row + buffer->cursor_row + buffer->row_offset - buffer->yscroll);

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
        }


        // Buffer title.
        leaf_draw_rect
        (
            nmt_coltox(nemi, buffer->position_col),
            nmt_rowtoy(nemi, buffer->position_row),
            buffer->max_col * nemi->font.char_width,
            nemi->font.char_height + 2,
            (struct color_t) {
                20, 20, 20
            }
        );



        // Border.
        {
            struct color_t border_color = (struct color_t){ 20, 80, 50 };
            struct color_t border_color_dim = (struct color_t){ 10, 30, 25 };
            
            // Top.
            leaf_draw_rect
            (
                nmt_coltox(nemi, buffer->position_col) - 2,
                nmt_rowtoy(nemi, buffer->position_row) - 2,
                buffer->max_col * nemi->font.char_width + 2,
                1,
                border_color
            );

            // Bottom.
            leaf_draw_rect
            (
                nmt_coltox(nemi, buffer->position_col) - 2,
                nmt_rowtoy(nemi, buffer->position_row + buffer_screen_max_row(buffer)+1) + 8,
                buffer->max_col * nemi->font.char_width + 2,
                1,
                border_color_dim
            );

            // Left.
            leaf_draw_rect
            (
                nmt_coltox(nemi, buffer->position_col) - 2,
                nmt_rowtoy(nemi, buffer->position_row) - 2,
                1,
                nmt_rowtoy(nemi, buffer_screen_max_row(buffer)+1),
                border_color
            );

            // Right.
            leaf_draw_rect
            (
                nmt_coltox(nemi, buffer->position_col + buffer->max_col),
                nmt_rowtoy(nemi, buffer->position_row) - 2,
                1,
                nmt_rowtoy(nemi, buffer_screen_max_row(buffer)+1),
                border_color_dim
            );
        }


        if(buffer->flags & BUFFER_INTERACTABLE) {
            nemi->font.char_color_r = 0.4f;
            nemi->font.char_color_g = 0.4f;
            nemi->font.char_color_b = 0.4f;

            leaf_draw_text
            (
                &nemi->font,
                nmt_coltox(nemi, buffer->position_col + buffer->max_col - 3),
                nmt_rowtoy(nemi, buffer->position_row),
                "(x)", 3
            );
        }



        char title_str[32] = { 0 };
        const char* buffer_title = buffer->title == NULL ? "" : buffer->title;
        nemi->font.char_color_r = 0.3f;
        nemi->font.char_color_g = 0.7f;
        nemi->font.char_color_b = 0.4f;

        switch(buffer->input_mode) {
        
            case IMODE_INSERT:
                snprintf(title_str, sizeof(title_str)-1,
                        "%s%s", 
                        buffer_title,
                        (buffer->flags & BUFFER_NOMODE_INTITLE) ? "" : " [insert]");
                break;

            case IMODE_VIEW:
                snprintf(title_str, sizeof(title_str)-1,
                        "%s%s", 
                        buffer_title,
                        (buffer->flags & BUFFER_NOMODE_INTITLE) ? "" : " [view]");
                break;

                // ...
        }

        leaf_draw_text
        (
            &nemi->font,
            nmt_coltox(nemi, buffer->position_col),
            nmt_rowtoy(nemi, buffer->position_row),
            title_str, strlen(title_str)
        );
    }
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


void module_event_window_resized() {
    TXModest* txmst = get_txmst();
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* buffer = txmst->buffers[i];
        if(buffer == NULL) {
            continue;
        }

        if(buffer->is_full_max_row) {
            buffer->max_row = txmst->term->rows;
        }
        
        if(buffer->is_full_max_col) {
            buffer->max_col = txmst->term->cols;
        }
    }
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


