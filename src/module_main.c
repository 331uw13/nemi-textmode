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


// This function is for select region callback.
static
size_t p_select_callback_buffer_get_row_length(void* user_pointer, ssize_t row) {
    Buffer* buf = (Buffer*)user_pointer;
    Bufrow* bufrow = buffer_get_row(buf, row);
    return (bufrow == NULL) ? 0 : bufrow->len;
}

static
int p_get_status_line_ypos(Nemi* nemi) {
    return nmt_rowtoy(nemi, nemi->win_rows-1);
}

static
void p_draw_status_line_box(Nemi* nemi, Buffer* buf) { 
    int status_line_y = p_get_status_line_ypos(nemi);
    leaf_draw_rect
    (
        nmt_coltox(nemi, 0),
        status_line_y,
        buf->max_col * nemi->font.char_width,
        nemi->font.char_height + 2,
        (RGBColor) {
            13, 13, 13
        }
    );
}

static
void p_draw_status_line_text(Nemi* nemi, Buffer* buf) { 
    const int max_width = 14;
    const int box_spacing = 10;
    const bool scroll_max = true;
    const float scroll_interval_sec = 0.15f;
    const float scroll_delay_restart_sec = 0.3f;

    const RGBColor active_box_color   = (RGBColor) { 32, 32, 32 };
    const RGBColor deactive_box_color = (RGBColor) { 23, 23, 23 };

    const RGBColor active_text_color   = (RGBColor) { 120, 120, 120 };
    const RGBColor deactive_text_color = (RGBColor) { 80, 80, 80 };

    int draw_x = nmt_coltox(nemi, 0);
    int draw_y = p_get_status_line_ypos(nemi);

    TXModest* txmst = get_txmst();


    for(size_t i = 0; i < MAX_BUFFERS; i++) {
        Buffer* iterbuf = txmst->buffers[i];
        if(iterbuf == NULL) {
            continue;
        }


        bool scroll_bufname = iterbuf->status_name.name_len_original > max_width;
        if(scroll_bufname
        && (iterbuf->status_name.scroll_timer_delay_restart_sec >= scroll_delay_restart_sec)) {

            //iterbuf->status_name.name_len = max_width;
            iterbuf->status_name.scroll_timer_sec += (float)nemi->frame_time;


            bool reached_end = (iterbuf->status_name.name_len - 1 < max_width / 2);
            bool allow_restart = (iterbuf->status_name.scroll_timer_delay_restart_sec >= scroll_delay_restart_sec * 2.0f);

            if(iterbuf->status_name.scroll_timer_sec >= scroll_interval_sec) {
                iterbuf->status_name.scroll_timer_sec = 0.0f;
  
                if(reached_end && allow_restart) {
                    iterbuf->status_name.name_ptr = iterbuf->name;
                    iterbuf->status_name.name_len = iterbuf->status_name.name_len_original;
                    iterbuf->status_name.scroll_timer_delay_restart_sec = 0;
                }
                else
                if(!reached_end) {
                    iterbuf->status_name.name_ptr++;
                    iterbuf->status_name.name_len--;
                }

            }
           
            if(reached_end && !allow_restart) {
                iterbuf->status_name.scroll_timer_delay_restart_sec += (float)nemi->frame_time;
            }

            /*
            if(reached_end) {
                iterbuf->status_name.scroll_timer_delay_restart_sec += (float)nemi->frame_time;
            }
            */
        }
        else
        if(scroll_bufname) {
            iterbuf->status_name.scroll_timer_delay_restart_sec += (float)nemi->frame_time;
        }
        /*
        else {
            iterbuf->status_name.name_len = iterbuf->status_name.name_len;
        }
        */


        leaf_draw_rect
        (
            draw_x,
            draw_y,
            nemi->font.char_width * max_width,
            nemi->font.char_height,
            (iterbuf == buf) ? active_box_color : deactive_box_color
        );

        leaf_set_font_color(&nemi->font,
                (iterbuf == buf) ? active_text_color : deactive_text_color);

        leaf_draw_text
        (
            &nemi->font,
            draw_x,
            draw_y,
            iterbuf->status_name.name_ptr, 
            MIN_VALUE(iterbuf->status_name.name_len, max_width)
        );



        draw_x += max_width * nemi->font.char_width + box_spacing;
    }
}


static
void p_draw_cmd_line(Nemi* nemi, TXModest* txmst) {
    int status_line_y = p_get_status_line_ypos(nemi);

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

static
void p_draw_buffer_cursor(Nemi* nemi, Buffer* buf) {
    int cursor_x = nmt_coltox(nemi, buf->cursor_col + buf->col_offset);
    int cursor_y = nmt_rowtoy(nemi, buf->cursor_row + buf->row_offset - buf->yscroll);
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
}

static
void p_draw_buffer_title_box(Nemi* nemi, Buffer* buf) {
    leaf_draw_rect
    (
        nmt_coltox(nemi, 0),
        nmt_rowtoy(nemi, 0),
        buf->max_col * nemi->font.char_width,
        nemi->font.char_height + 2,
        (RGBColor) {
            20, 20, 20
        }
    );
}

static
void p_draw_buffer_title_text(Nemi* nemi, Buffer* buf) {
    char buffer_title[256] = {0};
    ssize_t buffer_title_len = 0;
    const char* buffer_name = buf->name == NULL ? "" : buf->name;

    switch(buf->input_mode) {
    
        case IMODE_INSERT:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buf->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [insert]");
            break;

        case IMODE_VIEW:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buf->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [view]");
            break;


        case IMODE_SELECT:
            buffer_title_len 
                = snprintf(buffer_title, sizeof(buffer_title)-1,
                    "%s%s", 
                    buffer_name,
                    (buf->flags & BUFFER_NO_MODE_IN_TITLE) ? "" : " [select]");
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
 
}

void module_event_render() {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    if(!txmst->enabled) {
        return;
    }

    
    Buffer* buf = txmst->buffer;


    if(buf == NULL) {
        char* message = "No buffer opened. Press <?> for help ...";
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


    p_draw_status_line_box(nemi, buf);

    if(txmst->cmd_line_enabled) {
        p_draw_cmd_line(nemi, txmst);
    }
    else {
        p_draw_status_line_text(nemi, buf);
    }


    // Initiate select region draw.
    if(txmst->buffer->input_mode == IMODE_SELECT) {
        nmt_select_process
        (
            txmst->buffer->select,
            p_select_callback_buffer_get_row_length,
            buffer_select_render_callback,
            (void*)txmst->buffer // user pointer.
        );
    }


    p_draw_buffer_cursor(nemi, buf);
    p_draw_buffer_title_box(nemi, buf);
    p_draw_buffer_title_text(nemi, buf);
   
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
        Buffer* buf = txmst->buffers[i];
        if(buf == NULL) {
            continue;
        }

        buf->max_row = txmst->term->rows;
        buf->max_col = txmst->term->cols;
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


// NOTE: temporary function-
static
void p_append_bufrow(Buffer* buf, char* data) {
    Bufrow* row = buffer_insert_row(buf, buf->num_rows);
    bufrow_set(row, data, strlen(data));
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
    Buffer* test = add_new_buffer("test", IMODE_INSERT, BUFFER_NO_FLAGS);
    test->undostack.flags |= UNDOSTACK_IGNORE_PUSH;
    p_append_bufrow(test, "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    p_append_bufrow(test, "Vestibulum viverra felis ac metus finibus iaculis.");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "Curabitur dictum ante magna, id semper massa rhoncus ut.");
    p_append_bufrow(test, "Maecenas ac eros dui. Phasellus quis augue porta, tincidunt ligula in,");
    p_append_bufrow(test, "tempus metus. Integer et rutrum velit. Nulla facilisi.");
    p_append_bufrow(test, "Duis elementum felis vestibulum, vestibulum est sed,");
    p_append_bufrow(test, "eleifend ipsum. Etiam varius felis tempor, iaculis purus et,");
    p_append_bufrow(test, "sagittis nibh. Vestibulum sapien urna,");
    p_append_bufrow(test, "varius et dolor bibendum");
    p_append_bufrow(test, "rutrum posuere sapien");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "Nullam in metus odio.");
    p_append_bufrow(test, "Nunc ac commodo elit");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "vel rutrum ligula.  Fusce ultrices lacus aliquet");
    p_append_bufrow(test, "eros pulvinar commodo.");
    p_append_bufrow(test, "Suspendisse potenti.");
    p_append_bufrow(test, "Integer consequat nisiquis posuere consectetur.");
    p_append_bufrow(test, "Duis semper commodo magna.");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "");
    p_append_bufrow(test, "Nullam at arcu ut quam pharetra molestie.");
    p_append_bufrow(test, "Etiam egestas interdum nisl");
    p_append_bufrow(test, "eget placerat libero");
    p_append_bufrow(test, "Morbi facilisis leo eget ex fermentum consectetur");
    p_append_bufrow(test, "Sed molestie vehicula diam dignissim imperdiet");
    test->undostack.flags &= ~UNDOSTACK_IGNORE_PUSH;

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

