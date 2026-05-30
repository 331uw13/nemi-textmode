#include <assert.h>

#include "buffer.h"
#include "text_mode.h"

#include "nemi.h"
#include "fileio.h"

#include <stdlib.h>
#include <stdio.h>



/*
static
void print_buffer_linked_list(Buffer* buf) {
    
    printf("\n");
    printf("##### %s #####\n\n", __func__);

    for(size_t i = 0; i < buf->num_row_nodes; i++) {
        Bufrow* row = &buf->rows[i];

        printf("Row --- [\033[34m%p\033[0m] %li\n", row, i);
        printf("   Prev: \033[35m%p\033[0m\n", row->prev);
        printf("   Next: \033[35m%p\033[0m\n", row->next);
        printf("\n");

    }

    printf("#############################\n\n");
}

*/
/*
static // Temporary function.
void validate_linked_list(Buffer* buf) {
    bool error = false;
    Bufrow* row = buf->rows_head;
    while(row) {

        printf("[\033[34m%p\033[0m] ", row);
        bool tailorhead = false;
        if(row->prev == NULL) {
            printf("HEAD");
            tailorhead = true;
        }
        else {
            if(row->prev->next != row) {
                error = true;
                break;
            }
        }

        if(row->next == NULL) {
            printf("TAIL");
            tailorhead = true;
        }
        else {
            if(row->next->prev != row) {
                error = true;
                break;
            }

        }

        if(!tailorhead) {
            printf("....");
        }


        printf(" | ");

        if(row->data) {
            for(size_t l = 0; l < row->len; l++) {
                printf("%c", row->data[l]);
            }
        }

        printf("\n");
        row = row->next;
    }
    
    if(error) {
        printf("\033[1;31m Corrupted linked list\033[0m\n");
    }
    else {
        printf("\033[1;32m + Linked list validated\033[0m\n");
    }
    printf("\n");
}
*/


Buffer* buffer_init() {
    Buffer* buf = malloc(sizeof *buf);
    buf->rows = malloc(sizeof *buf->rows);
    bufrow_allocate(&buf->rows[0], buf);
    buf->rows_head = &buf->rows[0];
    buf->rows_tail = &buf->rows[0];
    buf->flags = 0;
    //buf->num_row_nodes = 0;
    //buf->num_row_nodes_alloc = 0;
    buf->num_rows = 1;
    buf->cursor_col = 0;
    buf->cursor_row = 0;
    buf->row_offset = 1; // Leave room for title bar.
    buf->col_offset = 0;
    buf->opened_file_path = NULL;
    buf->yscroll = 0;
    buf->input_mode = IMODE_INSERT;

    buf->settings.tab_width_in = 4;
    buf->settings.tab_width_out = 4;

    TXModest* txmst = get_txmst();
    buf->max_row = txmst->term->rows;
    buf->max_col = txmst->term->cols;

    buf->undostack = undostack_init();

    //buffer_insert_row(buf, 0);

    //print_buffer_linked_list(buf);
    //validate_linked_list(buf);
    return buf;
}


static
void p_buffer_free_all_rows(Buffer* buf) {
    Bufrow* row = buf->rows_tail;
    while(row) {
        Bufrow* prev = row->prev;
        
        //asm("int3");
        bufrow_free(row);


        if(row->next != NULL) {
            free(row->next);
        }

        row = prev;
    }
    free(buf->rows);
}

void buffer_free(Buffer* buf) {
    p_buffer_free_all_rows(buf);
    if(buf->name) {
        free(buf->name);
    }
    free(buf);
}

int buffer_screen_max_row(Buffer* buf) {
    return buf->max_row - buf->row_offset - 1;
}

int buffer_screen_max_col(Buffer* buf) {
    return buf->max_col - buf->col_offset;
}

void buffer_move_cursor_to(Buffer* buf, ssize_t row, ssize_t col) {
    Bufrow* bufrow = buffer_get_row(buf, row);
    if(bufrow == NULL) {
        return;
    }
    
    const bool moved_up   = buf->cursor_row > row;
    const bool moved_down = buf->cursor_row < row;

    // Save for undostack.
    const ssize_t cursor_row_old = buf->cursor_row;
    const ssize_t cursor_col_old = buf->cursor_col;

    buf->cursor_row = row;
    buf->cursor_col = col;

    if(buf->cursor_row < 0) {
        buf->cursor_row = 0;
    }
    else
    if(buf->cursor_row >= (ssize_t)buf->num_rows) {
        buf->cursor_row = buf->num_rows-1;
    }
    
    if(buf->cursor_col < 0) {
        buf->cursor_col = 0;
    }
    else
    if(buf->cursor_col > (ssize_t)bufrow->len) {
        buf->cursor_col = bufrow->len;
    }


    if(moved_down) {
        if(buf->cursor_row - buf->yscroll >= buffer_screen_max_row(buf)) {
            buffer_yscroll_to(buf, buf->cursor_row - buffer_screen_max_row(buf) + 1);
        }
    }
    else
    if(moved_up) {
        if(buf->cursor_row < buf->yscroll) {
            buffer_yscroll_to(buf, buf->cursor_row);
        }
    }
}

void buffer_move_cursor(Buffer* buf, int row_offset, int col_offset) {
    buffer_move_cursor_to(buf, 
            buf->cursor_row + row_offset,
            buf->cursor_col + col_offset);
}

void buffer_yscroll_to(Buffer* buf, ssize_t position) {
    if(position < 0) {
        return;
    }

    buf->yscroll = position;
}

void buffer_yscroll(Buffer* buf, int offset) {
    buffer_yscroll_to(buf, buf->yscroll + offset);
}


static
void p_buffer_fix_row_numbers(Buffer* buf, size_t position) {
    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return;
    }

    while(row) {
        row->number = (row->prev == NULL) ? 0 : (row->prev->number + 1);
        row = row->next;
    }
}

Bufrow* buffer_insert_row(Buffer* buf, size_t position) {
    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return NULL;
    }

    Bufrow* new_row = malloc(sizeof(Bufrow));
    bufrow_allocate(new_row, buf);

    new_row->prev = row;
    new_row->next = row->next;

    if(row->next) {
        row->next->prev = new_row;
    }

    row->next = new_row;

    bool at_tail = row == buf->rows_tail;
    if(at_tail) {
        buf->rows_tail = new_row;
    }

    buf->num_rows++;

    

    undostack_push(&buf->undostack,
    (UndoStackCmd) {
        .kind = UCMD_INSERT_ROW,
        .location = (UndoStackCmdLocation) {
            .buf_id = buffer_getid(buf),
            .row = row->number + 1,
            .col = 0
        }
    });

    p_buffer_fix_row_numbers(buf, position);
    //validate_linked_list(buf);
    //buf->num_row_nodes++;
    return new_row;
}

bool buffer_delete_row(Buffer* buf, size_t position) {
    if(buf->num_rows == 1) {
        return false;
    }
    
   // validate_linked_list(buf);

    Bufrow* row = buffer_get_row(buf, position);
    if(row == NULL) {
        return false;
    }
    
    // Save for undostack.
    size_t row_number   = row->number;
    size_t row_data_len = row->len;
    char* row_data      = bufrow_datadup(row); 


    if(row->next) {
        row->next->prev = row->prev;
    }
    else {
        buf->rows_tail = row->prev;
    }

    if(row->prev) {
        row->prev->next = row->next;
    }
    else {
        buf->rows_head = row->next;
    }

    
    // Set below rows to re-written to terminal.
    Bufrow* row_iter = row;
    while(row_iter) {
        row_iter->dirty = true;
        row_iter = row_iter->next;
    }

    row->next = NULL;
    row->prev = NULL;

    
    bufrow_free(row);
    row->data = NULL;
    row->len = 0;

    free(row);
    //printf("Row Data: %p\n", row->data);

    //validate_linked_list(buf);
    //printf("Remove %p  \033[33mTODO: Nodes may become fragmented.\033[0m\n", row);
    buf->num_rows--;


    p_buffer_fix_row_numbers(buf, position);

    undostack_push(&buf->undostack,
    (UndoStackCmd) {
        .kind = UCMD_DELETE_ROW,
        .location = (UndoStackCmdLocation) {
            .buf_id = buffer_getid(buf),
            .row = row_number,
            .col = 0
        },
        .data_as.char_array = row_data,
        .data_len = row_data_len
    });


    return true;
}

void buffer_delete_all_rows(Buffer* buf) {
    buffer_clean_visible_rows(buf);
    p_buffer_free_all_rows(buf);
    
    buf->rows = malloc(sizeof *buf->rows);
    bufrow_allocate(&buf->rows[0], buf);
    buf->rows_head = &buf->rows[0];
    buf->rows_tail = &buf->rows[0];
    
    buf->cursor_col = 0;
    buf->cursor_row = 0;
    buf->yscroll = 0;
    buf->num_rows = 1;
}


Bufrow* buffer_get_row(Buffer* buf, ssize_t position) {
    if(position < 0 || position > (ssize_t)buf->num_rows + 1) {
        return NULL;
    }
    Bufrow* row = buf->rows_head;

    for(ssize_t i = 0; i < position; i++) {
        if(!row->next) { break; }
        row = row->next;
    }

    return row;
}


void buffer_eventkey_enter(Buffer* buf, Bufrow* row) {
    undostack_start_new_group(&buf->undostack);
    
    if(!buffer_insert_row(buf, buf->cursor_row)) {
        return;
    }

    int row_indent = bufrow_count_indent(row);

    // This is the new inserted row.
    Bufrow* row_below = row->next; 

    if(buf->cursor_col < (ssize_t)row->len) {
        BufrowSubstr substr = bufrow_substr_p(row, buf->cursor_col, row->len);
    
        bufrow_insert_substr(row_below, 0, substr);
        bufrow_cut(row, buf->cursor_col, row->len - buf->cursor_col);
    }
    

    for(int i = 0; i < row_indent; i++) {
        bufrow_insert_char(row_below, 0, ' ');
    }

    buffer_move_cursor_to(buf, buf->cursor_row+1, row_indent);
}

void buffer_eventkey_backspace(Buffer* buf, Bufrow* row) {
    undostack_start_new_group(&buf->undostack);
    
    if(buf->cursor_col > 0) {
        bufrow_delete_char(row, buf->cursor_col-1);
        buffer_move_cursor(buf, 0, -1);
        return;
    }

    if(buf->cursor_row <= 0) {
        return;
    }

    size_t above_row_old_len = row->prev == NULL ? 0 : row->prev->len;
    if(row->prev != NULL) {
        BufrowSubstr substr = bufrow_substr_p(row, 0, row->len);
        bufrow_insert_substr(row->prev, row->prev->len, substr);
    }

    buffer_delete_row(buf, buf->cursor_row);
    buffer_move_cursor_to(buf, buf->cursor_row-1, above_row_old_len);
}

void buffer_clean_visible_rows(Buffer* buf) {
    ssize_t row_counter = 0;
    Bufrow* row = buffer_get_row(buf, buf->yscroll);
    
    while(row) {
        int real_row = buf->row_offset + row_counter;
        nmterm_clear_row_part(get_txmst()->term, real_row, 0, buf->max_col);
        

        row->dirty = true; // Set to dirty for next update so the row wont stay hidden.
        row = row->next;

        row_counter++;
        if(row_counter >= buffer_screen_max_row(buf)) {
            break;
        }
    }
}


void buffer_write_to_terminal(Buffer* buf) {
    NTerminal* term = get_txmst()->term;

    ssize_t row_counter = 0;
    Bufrow* row = buffer_get_row(buf, buf->yscroll);
   


    char linenum_buf[32] = { 0 };

    if(!(buf->flags & BUFFER_NO_NUMBER)) {
        buf->col_offset = snprintf(linenum_buf, sizeof(linenum_buf)-1,
                "%li", buf->num_rows) + 1;
    }

    //nmterm_clear_row_part(g_txmst->term, );
    //nmterm_clear_row(g_txmst->term, buf->position_col + buf->num_rows+1 - buf->yscroll);
    nmterm_clear_row_part(term, 
            buf->num_rows+1 - buf->yscroll, 0, buf->max_col);


    while(row) {

        /*
        if(buf->flags & BUFFER_CLEAN_VISIBLE_ROWS) {
            row->dirty = true;
        }
        */

        if(row->dirty) {
            int real_row = buf->row_offset + row_counter;
            
            nmterm_clear_row_part(term, real_row, 0, buf->max_col);


            if(!(buf->flags & BUFFER_NO_NUMBER)) {
                ssize_t row_number = row_counter + buf->yscroll;
                ssize_t linenum_buf_len 
                    = snprintf(linenum_buf, sizeof(linenum_buf)-1, "\033[90m%li\033[0m", row->number);

                nmterm_mv_putstrn
                (
                    term,
                    real_row,
                    0,
                    linenum_buf,
                    linenum_buf_len
                );
            }

            for(size_t col = 0; col < row->len; col++) {

                nmterm_mv_putchr
                (
                    term,
                    real_row,
                    buf->col_offset + col,
                    row->data[col]
                );
            }
        }

        row = row->next;
        row_counter++;

        if(row_counter >= buffer_screen_max_row(buf)) {
            break;
        }
    }
        
    //buf->flags &= ~BUFFER_CLEAN_VISIBLE_ROWS;
}


void buffer_read_file(Buffer* buf, const char* file_path) {

    char* file = NULL;
    size_t file_size = 0;

    if(!map_file(file_path, PROT_READ, &file, &file_size)) {
        return;
    }


    Bufrow* row = buffer_get_row(buf, 0);

    char* c = file;
    char* end = file + file_size;

    size_t num_nonascii = 0;

    while(c < end) {
   

        if(*c == '\n') {
            row = buffer_insert_row(buf, buf->num_rows);
        }
        else
        if(*c == '\t') {
            for(int i = 0; i < buf->settings.tab_width_in; i++) {
                bufrow_insert_char(row, row->len, ' ');
            }
        }
        else
        if(*c >= 0x20 && *c <= 0x7E) {
            bufrow_insert_char(row, row->len, *c);
        }
        else {
            num_nonascii++;
        }

        c++;
    }

    if(buf->opened_file_path != NULL) {
        free(buf->opened_file_path);
    }

    buf->opened_file_path = strdup(file_path);


    if(num_nonascii > 0) {
        logprintf(LOG_WARN, "%li Non-ASCII character was hit, when reading a file.", num_nonascii);
    }
}

void buffer_save_file(Buffer* buf) {
}



void buffer_select_render_callback(void* userptr, ssize_t row, ssize_t row_length, ssize_t col_begin) {
    if(row_length <= 0) {
        row_length = 1;
    }

    Nemi* nemi = nmt_getst();
    Buffer* buf = (Buffer*)userptr;

    leaf_draw_rect  
    (
        nmt_coltox(nemi, buf->col_offset + col_begin),   
        nmt_rowtoy(nemi, buf->row_offset + row - buf->yscroll),
        nemi->font.char_width * row_length,
        nemi->font.char_height + nemi->cfg.main.line_padding,
        nemi->cfg.colors[NEMI_COLOR_TERM_SELECT_REG]   
    );
}

