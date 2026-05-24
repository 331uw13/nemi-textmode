#include <stdio.h>
#include <math.h>

#include "nemi.h"
#include "text_mode.h"
#include "cmd_line.h"



static const int KEYBIND_TOGGLE[] = {
    GLFW_KEY_U,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_BROWSE_FILES[] = {
    GLFW_KEY_F,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_NEXT_BUFFER[] = {
    GLFW_KEY_N,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_PREV_BUFFER[] = {
    GLFW_KEY_P,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_BUFFER_IMODE_VIEW[] = {
    GLFW_KEY_X,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_BUFFER_IMODE_INSERT[] = {
    GLFW_KEY_W,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_BUFFER_IMODE_SELECT[] = {
    GLFW_KEY_S,
    GLFW_KEY_LEFT_CONTROL
};

static const int KEYBIND_CMDLINE_OPEN[] = {
    GLFW_KEY_M,
    GLFW_KEY_LEFT_CONTROL
};

/*
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
*/


static
size_t p_select_callback_buffer_get_row_length(void* user_pointer, ssize_t row) {
    Buffer* buf = (Buffer*)user_pointer;
    Bufrow* bufrow = buffer_get_row(buf, row);
    return (bufrow == NULL) ? 0 : bufrow->len;
}


void module_event_render() {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }



    // NOTE: This needs to be organized later.

    Buffer* buffer = txmst->buffer;

    if(buffer == NULL) {
        const char* message = "No buffer opened. Press <?> for help ...";


        leaf_set_font_color(&nemi->font, (RGBColor){ 150, 150, 150 });
        leaf_draw_text
        (
            &nemi->font,
            10,
            10,
            message, 
            strlen(message)
        );
        return;
    }


    int status_line_y = nmt_rowtoy(nemi, nemi->win_rows-1);
 
    // Status line box
    leaf_draw_rect
    (
        nmt_coltox(nemi, 0),
        status_line_y,
        buffer->max_col * nemi->font.char_width,
        nemi->font.char_height + 2,
        (RGBColor) {
            13, 13, 13
        }
    );

    if(txmst->cmd_line_enabled) {

        // Cursor
        int cursor_x = nmt_coltox(nemi, txmst->cmd_line_cursor);

        leaf_draw_rect
        (
            cursor_x,
            status_line_y,
            nemi->font.char_width,
            nemi->font.char_height,
            (RGBColor) {
                80, 20, 30
            }
        );


        leaf_set_font_color(&nemi->font, (RGBColor){ 130, 50, 80 });
        leaf_draw_text
        (
            &nemi->font,
            nmt_coltox(nemi, 0),
            status_line_y,
            txmst->cmd_str.bytes, 
            txmst->cmd_str.size
        );
    }
    else {
        
    }

    if(txmst->buffer->input_mode == IMODE_SELECT) {
        nmt_select_process
        (
            txmst->buffer->select,
            p_select_callback_buffer_get_row_length,
            buffer_select_render_callback,
            (void*)txmst->buffer // user pointer.
        );
    }

    // Cursor
    int cursor_x = nmt_coltox(nemi, 
            buffer->cursor_col + buffer->col_offset);

    int cursor_y = nmt_rowtoy(nemi, 
            buffer->cursor_row + buffer->row_offset - buffer->yscroll);

    leaf_draw_rect
    (
        cursor_x,
        cursor_y,
        nemi->font.char_width,
        nemi->font.char_height,
        (RGBColor) {
            10, 60, 60
        }
    );

    // Buffer title box.
    leaf_draw_rect
    (
        nmt_coltox(nemi, 0),
        nmt_rowtoy(nemi, 0),
        buffer->max_col * nemi->font.char_width,
        nemi->font.char_height + 2,
        (RGBColor) {
            20, 20, 20
        }
    );


    // Title.

    char buffer_title[32] = {0};
    ssize_t buffer_title_len = 0;
    const char* buffer_name = buffer->name == NULL ? "" : buffer->name;



    switch(buffer->input_mode) {
    
        case IMODE_INSERT:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buffer->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [insert]");
            break;

        case IMODE_VIEW:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buffer->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [view]");
            break;


        case IMODE_SELECT:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buffer->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [select]");
            break;


        default:
            buffer_title_len
                = snprintf(buffer_title, sizeof(buffer_title)-1, "@%s", buffer_name);
            break;
            // ...
    }


    if(buffer_title_len > 0) {

        leaf_set_font_color(&nemi->font, (RGBColor){ 150, 120, 180 });
        leaf_draw_text
        (
            &nemi->font,
            nmt_coltox(nemi, 0),
            nmt_rowtoy(nemi, 0),
            buffer_title, buffer_title_len
        );
    }
    
    
    if(txmst->update_buffers) {
        update_buffers();
    }
}


void module_event_key_input(int key, int mods) {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    if(txmst->cmd_line_enabled) {
        handle_cmd_line_keypress(key, mods);
        return;
    }

    if(!txmst->buffer) {
        return;
    }


    IModeCallbacks* imode_calls = &txmst->imode_callbacks[txmst->buffer->input_mode];
    
    if(imode_calls->buffer_keypress) {
        imode_calls->buffer_keypress(txmst->buffer, key, mods);
    }


    txmst->update_buffers = true;
}

void module_event_char_input(char ch) {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    if(ch < 0x20 || ch > 0x7E) {
        return; // Dont accept non-ascii characters.
    }


    if(txmst->cmd_line_enabled) {
        handle_cmd_line_chrpress(ch);
        return;
    }

    if(!txmst->buffer) {
        return;
    }


    IModeCallbacks* imode_calls = &txmst->imode_callbacks[txmst->buffer->input_mode];
    
    if(imode_calls->buffer_chrpress) {
        imode_calls->buffer_chrpress(txmst->buffer, ch);
    }


    txmst->update_buffers = true;
}


void module_event_window_resized() {
    TXModest* txmst = get_txmst();
    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* buffer = txmst->buffers[i];
        if(buffer == NULL) {
            continue;
        }

        buffer->max_row = txmst->term->rows;
        buffer->max_col = txmst->term->cols;
    }
}

void kb_toggle_module() {
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


void kb_open_file_browsing() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }


    add_new_buffer("files", IMODE_FILES, BUFFER_IMODE_CANT_CHANGE);
}


void kb_switch_to_next_buffer() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }
    for(size_t i = txmst->buffer->index+1; i < MAX_BUFFERS; i++) {
        Buffer* buf = txmst->buffers[i];
        if(buf != NULL) {
            switch_to_buffer(buf);
            break;
        }
    }
}

void kb_switch_to_prev_buffer() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    for(ssize_t i = txmst->buffer->index-1; i >= 0; i--) {
        Buffer* buf = txmst->buffers[i];
        if(buf != NULL) {
            switch_to_buffer(buf);
            break;
        }
    }
}

void kb_buffer_imode_insert() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }
    if(txmst->buffer->flags & BUFFER_IMODE_CANT_CHANGE) {
        return;
    }

    txmst->buffer->input_mode = IMODE_INSERT;
}

void kb_buffer_imode_view() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }
    if(txmst->buffer->flags & BUFFER_IMODE_CANT_CHANGE) {
        return;
    }

    txmst->buffer->input_mode = IMODE_VIEW;
}

void kb_buffer_imode_select() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }
    if(txmst->buffer->flags & BUFFER_IMODE_CANT_CHANGE) {
        return;
    }

    txmst->buffer->input_mode = IMODE_SELECT;
    nmt_select_begin(
            &txmst->buffer->select,
            txmst->buffer->cursor_col,
            txmst->buffer->cursor_row);
}

void kb_cmd_line_open() {
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    txmst->cmd_line_enabled = true;
}



void module_loaded(size_t module_idx) {
    create_txmst();
    TXModest* txmst = get_txmst();
    txmst->enabled = false;
    txmst->module_idx = module_idx;
    
    Nemi* nemi = nmt_getst();
    

    txmst->imode_callbacks[IMODE_INSERT] = (IModeCallbacks) {
        .buffer_added    = NULL,
        .buffer_free     = NULL,
        .buffer_keypress = imode_INSERT_keypress,
        .buffer_chrpress = imode_INSERT_chrpress
    };

    txmst->imode_callbacks[IMODE_VIEW] = (IModeCallbacks) {
        .buffer_added    = NULL,
        .buffer_free     = NULL,
        .buffer_keypress = imode_VIEW_keypress,
        .buffer_chrpress = imode_VIEW_chrpress
    };

    txmst->imode_callbacks[IMODE_SELECT] = (IModeCallbacks) {
        .buffer_added    = NULL,
        .buffer_free     = NULL,
        .buffer_keypress = imode_SELECT_keypress,
        .buffer_chrpress = imode_SELECT_chrpress
    };

    txmst->imode_callbacks[IMODE_FILES] = (IModeCallbacks) {
        .buffer_added    = imode_FILES_buffer_added,
        .buffer_free     = imode_FILES_buffer_free,
        .buffer_keypress = imode_FILES_keypress,
        .buffer_chrpress = imode_FILES_chrpress
    };
    
    //add_new_buffer("files", IMODE_FILES, BUFFER_IMODE_CANT_CHANGE);
    add_new_buffer("test", IMODE_INSERT, BUFFER_NO_FLAGS);

    nmt_assign_module_keybind
    (
        nemi, 
        module_idx,
        kb_toggle_module,
        KEYBIND_TOGGLE, 
        ARRAY_LEN(KEYBIND_TOGGLE)
    );
   
    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_open_file_browsing, 
        KEYBIND_BROWSE_FILES,
        ARRAY_LEN(KEYBIND_BROWSE_FILES)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_switch_to_next_buffer, 
        KEYBIND_NEXT_BUFFER,
        ARRAY_LEN(KEYBIND_NEXT_BUFFER)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_switch_to_prev_buffer, 
        KEYBIND_PREV_BUFFER,
        ARRAY_LEN(KEYBIND_PREV_BUFFER)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_buffer_imode_insert, 
        KEYBIND_BUFFER_IMODE_INSERT,
        ARRAY_LEN(KEYBIND_BUFFER_IMODE_INSERT)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_buffer_imode_view, 
        KEYBIND_BUFFER_IMODE_VIEW,
        ARRAY_LEN(KEYBIND_BUFFER_IMODE_VIEW)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_buffer_imode_select, 
        KEYBIND_BUFFER_IMODE_SELECT,
        ARRAY_LEN(KEYBIND_BUFFER_IMODE_SELECT)
    );

    nmt_assign_module_keybind
    (
        nemi,
        module_idx, 
        kb_cmd_line_open, 
        KEYBIND_CMDLINE_OPEN,
        ARRAY_LEN(KEYBIND_CMDLINE_OPEN)
    );
}


void module_quit() {
    free_txmst();
}

