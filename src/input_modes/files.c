#include "../input_modes.h"
#include "../text_mode.h"

#include "nemi.h"
#include "fileio.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>



typedef enum FilesSortOption_e {
    F_SORT_ALPHABETICAL,
    F_SORT_HIDDEN_LAST,
}
FilesSortOption;



typedef struct FilesBufferData_t {
    FileInfo* files;
    size_t    num_files;
}
FilesbufData;

/*
static
void p_copy_file_info(FileInfo* from, FileInfo* to) {
    to->sb = from->sb;
    memcpy(to->name, from->name, sizeof(from->name));
    memcpy(to->full_path, from->full_path, sizeof(from->full_path));
}

static
void p_swap_file_info(FileInfo* finfo_A, FileInfo* finfo_B) {
    FileInfo tmp = {0};
    p_copy_file_info(finfo_B, &tmp);
    p_copy_file_info(finfo_A, finfo_B);
    p_copy_file_info(&tmp, finfo_A);
}
*/





int qsort_alphabetical_order(const void* a, const void* b) {
    const FileInfo* finfo_a = (const FileInfo*)a;
    const FileInfo* finfo_b = (const FileInfo*)b;


    char first_lower_a = tolower(finfo_a->name[0]);
    char first_lower_b = tolower(finfo_b->name[0]);
    
    return first_lower_a - first_lower_b;
}

int qsort_hidden_last(const void* a, const void* b) {
    const FileInfo* finfo_a = (const FileInfo*)a;
    const FileInfo* finfo_b = (const FileInfo*)b;


    char first_a = finfo_a->name[0];
    char first_b = finfo_b->name[0];
    
    return (first_a == '.' && first_b != '.') ? 1 : 0;
}





static
void p_sort_files(FileInfo* files, size_t num_files, FilesSortOption* options, size_t num_options) {


    for(size_t i = 0; i < num_options; i++) {
    //qsort(files, num_files, sizeof *files, qsort_test);
    
        int(*qsort_callback)(const void*, const void*) = NULL;

        FilesSortOption opt = options[i];
        switch(opt) {

            case F_SORT_ALPHABETICAL:
                qsort_callback = qsort_alphabetical_order;
                break;

            case F_SORT_HIDDEN_LAST:
                qsort_callback = qsort_hidden_last;
                break;
        }
                
        if(qsort_callback != NULL) {
            qsort(files, num_files, sizeof *files, qsort_callback);
        }
    }
}


static
void p_list_files(Buffer* buf) {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    NModule* this_module = &nemi->modules[txmst->module_idx];

    FilesbufData* fg = (FilesbufData*)buf->user_pointer;
    
    if(fg->files != NULL) {
        buffer_delete_all_rows(buf);
        free(fg->files);
    }

    Bufrow* first_row = buffer_get_row(buf, 0); // The first row is always available.
    bufrow_set(first_row, this_module->current_dir, strlen(this_module->current_dir));



    fg->num_files = 0;
    fg->files = nmt_list_files(this_module->current_dir, &fg->num_files);
    if(fg->files == NULL) {
        return;
    }


    
    FilesSortOption sort_options[] = {
        F_SORT_ALPHABETICAL,
        F_SORT_HIDDEN_LAST
    };

    p_sort_files(fg->files, fg->num_files, sort_options, ARRAY_LEN(sort_options));


    for(size_t i = 0; i < fg->num_files; i++) {
        FileInfo* finfo = &fg->files[i];
        Bufrow* row = buffer_insert_row(buf, buf->num_rows);
    
        bufrow_set(row, finfo->name, strlen(finfo->name));
    

        const int indent = 5;
        BufrowSubstr type_str = (BufrowSubstr) {0};

        switch(finfo->sb.st_mode & S_IFMT) {
            case S_IFBLK:
                type_str.data_ptr = "blk";
                type_str.len = strlen(type_str.data_ptr);
                bufrow_insert_substr(row, 0, type_str);
                break;
            
            case S_IFREG:
                type_str.data_ptr = "reg";
                type_str.len = strlen(type_str.data_ptr);
                bufrow_insert_substr(row, 0, type_str);
                break;
            
            case S_IFDIR:
                type_str.data_ptr = ">";
                type_str.len = strlen(type_str.data_ptr);
                bufrow_insert_substr(row, 0, type_str);
                break; 
        
            default:
                type_str.data_ptr = "?";
                type_str.len = strlen(type_str.data_ptr);
                bufrow_insert_substr(row, 0, type_str);
                break;

        }

        for(size_t i = type_str.len; i < indent; i++) {
            bufrow_insert_char(row, type_str.len, ' ');
        }



    }
}


static
void p_open_selected_file(Buffer* buf) {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    NModule* this_module = &nemi->modules[txmst->module_idx];
    

    FilesbufData* fg = (FilesbufData*)buf->user_pointer;
    if(fg->files == NULL) {
        return;
    }

    if(buf->cursor_row <= 0 || buf->cursor_row >= fg->num_files) {
        return;
    }

    FileInfo* selected_finfo = &fg->files[buf->cursor_row - 1];
    
    int type = selected_finfo->sb.st_mode & S_IFMT;

    if(type == S_IFDIR) {
        if(nmt_module_chdir_abs(this_module, selected_finfo->full_path)) {
            p_list_files(buf);
        }
        else {
            logprintf(LOG_ERROR, "(textmode.so) Failed to access directory '%s'", 
                    selected_finfo->full_path);
        }
    }
    else
    if(type == S_IFREG) {
        logprintf(LOG_WARN, "TODO: Check if file cant be accessed.");

        Buffer* new_buffer = add_new_buffer(selected_finfo->name, IMODE_INSERT, BUFFER_NO_FLAGS);
        buffer_read_file(new_buffer, selected_finfo->full_path);
    }

    /*if((selected_finfo->sb.st_mode & S_IFMT) == S_IFDIR) {
        if(nmt_module_chdir_abs(this_module, selected_finfo->full_path)) {
            p_list_files(buf);
        }
        else {
            logprintf(LOG_ERROR, "(textmode.so) Failed to access directory '%s'", 
                    selected_finfo->full_path);
        }
    }
    */

}

void imode_FILES_buffer_added(Buffer* buf) {
    buf->user_pointer = calloc(1, sizeof(FilesbufData));

    p_list_files(buf);
}

void imode_FILES_buffer_free(Buffer* buf) {
    free(buf->user_pointer);
}

void imode_FILES_keypress(Buffer* buf, int key, int mods) {
    Nemi* nemi = nmt_getst();
    TXModest* txmst = get_txmst();
    NModule* this_module = &nemi->modules[txmst->module_idx];
    

    switch(key) {
        case GLFW_KEY_I:
            buffer_move_cursor(buf, -1, 0);
            break;
        
        case GLFW_KEY_K:
            buffer_move_cursor(buf, 1, 0);
            break;

        case GLFW_KEY_ENTER:
            p_open_selected_file(buf);
            break;
   
        case GLFW_KEY_BACKSPACE:
            if(nmt_module_chdir_rel(this_module, "..")) {
                p_list_files(buf);
            }
            break;
    }

}

void imode_FILES_chrpress(Buffer* buf, char c) {
    TXModest* txmst = get_txmst();


}
