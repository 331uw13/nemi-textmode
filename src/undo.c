#include <assert.h>
#include <stdlib.h>

#include "undo.h"
#include "buffer.h"
#include "text_mode.h"
#include "nemi.h"



UndoStack undostack_init() {
    return (UndoStack) {
        .cmd_groups = NULL,
        .cmd_groups_count = 0,
        .cmd_groups_alloc = 0,
        .flags = 0
    };
}


static
UndoStackCmdGroup* p_add_cmd_group(UndoStack* ub) {
    if(ub->cmd_groups_count + 1 >= ub->cmd_groups_alloc) {
        ub->cmd_groups_alloc = ub->cmd_groups_count + 64;
        ub->cmd_groups 
            = realloc
            (
                ub->cmd_groups,
                ub->cmd_groups_alloc * sizeof *ub->cmd_groups
            );
    }

    UndoStackCmdGroup* group = &ub->cmd_groups[ ub->cmd_groups_count ];
    ub->cmd_groups_count++;

    group->commands = NULL;
    group->commands_count = 0;
    group->commands_alloc = 0;

    return group;
}

static
void p_free_cmd_group(UndoStackCmdGroup* group) {

    // TODO: Need to free cmd.data_as.char_array (sometimes)

    free(group->commands);
}


void undostack_free(UndoStack* ub) {
}

bool undostack_isempty(UndoStack* ub) {
    return true;
}

void undostack_push(UndoStack* ub, UndoStackCmd cmd) {
    if(ub->flags & UNDOSTACK_IGNORE_PUSH) {
        return;
    }


    UndoStackCmdGroup* group = NULL;

    if(ub->cmd_groups == NULL || ub->cmd_groups_count == 0) {
        group = p_add_cmd_group(ub);
    }
    else {
        group = &ub->cmd_groups[ ub->cmd_groups_count - 1 ];
    }


    if(group->commands_count + 1 >= group->commands_alloc) {
        group->commands_alloc = group->commands_count + 32;
        group->commands
            = realloc
            (
                group->commands,
                group->commands_alloc * sizeof *group->commands
            );
    }

    group->commands[ group->commands_count ] = cmd;
    group->commands_count++;
}

void undostack_start_new_group(UndoStack* ub) {
    if(ub->cmd_groups_count == 0) {
        return;
    }

    UndoStackCmdGroup* group = &ub->cmd_groups[ ub->cmd_groups_count - 1 ];
    if(group->commands_count == 0) {
        return;
    }

    p_add_cmd_group(ub);
}

void undostack_finish_group(UndoStack* ub) {
    if(ub->cmd_groups_count == 0) {
        return;
    }

    UndoStackCmdGroup* group = &ub->cmd_groups[ ub->cmd_groups_count - 1 ];
    if(group->commands_count == 0) {
        return;
    }

    p_add_cmd_group(ub);
}

/*
void undostack_push(UndoStack* ub, UndoStackCmd cmd) {
    if(ub->flags & UNDOSTACK_IGNORE_PUSH) {
        return;
    }

    if(ub->commands_count + 1 >= ub->commands_alloc) {
        ub->commands_alloc = ub->commands_count + UBCOMMANDS_ALLOC_MORE;
        ub->commands
            = realloc
            (
                ub->commands,
                ub->commands_alloc * sizeof *ub->commands
            );
    }
    ub->commands[ub->commands_count++] = cmd;
}

UndoStackCmd undostack_pop(UndoStack* ub) {
    assert(!undostack_isempty(ub));
    UndoStackCmd ret_val = ub->commands[ub->commands_count - 1];
    ub->commands_count--;
    return ret_val;
}
*/


static
void p_undostack_execute_cmd(UndoStackCmd cmd) {

    Buffer* buf = NULL;
    
    // Figure out which buffer it is that executed this command.
    // Best case it will find the buffer right away,
    // Worst case it will loop through all buffers to see if uid match.
    // This solution is more safe than storing the buffer's address.
    // ^because this way we know if the buffer is actually alive anymore.
    {
        TXModest* txmst = get_txmst();
        Buffer* candidate_buf = txmst->buffers[ cmd.location.buf_id.buffer_index ];

        if(candidate_buf->uid == cmd.location.buf_id.buffer_uid) {
            buf = candidate_buf;
        }
        else {
            for(size_t i = 0; i < MAX_BUFFERS; i++) {
                candidate_buf = txmst->buffers[i];

                if(candidate_buf->uid == cmd.location.buf_id.buffer_uid) {
                    buf = candidate_buf;
                    break;
                }
            }
        }
    }

    if(buf == NULL) {
        logprintf(LOG_ERROR, "Failed to find buffer while executing undo stack command.");
        return;
    }
    
    // Set flag to ignore the undostack commands coming from the functions
    // which are used to execute the actions.
    buf->undostack.flags |= UNDOSTACK_IGNORE_PUSH;

    switch(cmd.kind) {

        case UCMD_CURSOR_MOVED:
            {
                buffer_move_cursor_to(buf,
                        cmd.location.row,
                        cmd.location.col);
            }
            break;

        case UCMD_INSERT_CHAR:
            {
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_delete_char(row, cmd.location.col);
            }
            break;
        
        case UCMD_DELETE_CHAR:
            {
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_insert_char(row, cmd.location.col, cmd.data_as.character);
            }
            break;

        case UCMD_INSERT_BLOCK:
            {
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_cut(row, cmd.location.col, cmd.data_len);
            }
            break;
        
        case UCMD_DELETE_BLOCK:
            {
                logprintf(LOG_INFO, "<insert block> %s", cmd.data_as.char_array);
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_insert_substr
                    (
                        row,
                        cmd.location.col,
                        (BufrowSubstr) {
                            .data_ptr = cmd.data_as.char_array,
                            .len = cmd.data_len
                        }
                    );
            }
            break;

        case UCMD_INSERT_ROW:
            {
                logprintf(LOG_INFO, "INSERT_ROW  <delete row> %li", cmd.location.row);
                buffer_delete_row(buf, cmd.location.row);
            }
            break;


        case UCMD_DELETE_ROW:
            {
                Bufrow* row = buffer_insert_row(buf, cmd.location.row - 1);
                if(row != NULL && cmd.data_as.char_array != NULL) {
                    bufrow_set(row, cmd.data_as.char_array, cmd.data_len);
                }
            }
            break;

        default:
            logprintf(LOG_WARN, "Executing undostack command but its not handled in switch statement.");
            break;
    }
    
    buffer_move_cursor_to(buf, cmd.location.row, cmd.location.col);
    buf->undostack.flags &= ~UNDOSTACK_IGNORE_PUSH;
}

void undostack_pop_and_exec_group(UndoStack* ub) {
    if(ub->cmd_groups == NULL || ub->cmd_groups_count == 0) {
        return;
    }

    UndoStackCmdGroup* group = &ub->cmd_groups[ ub->cmd_groups_count - 1 ];

    /*
    for(size_t i = 0; i < group->commands_count; i++) {
        p_undostack_execute_cmd(group->commands[i]);
    }
    */
   
    // Go backwards because push is actually just appending.
    for(ssize_t i = group->commands_count-1; i >= 0; i--) {
        p_undostack_execute_cmd(group->commands[i]);
    }

    p_free_cmd_group(group);
    group->commands = NULL;

    ub->cmd_groups_count -= 1;
}
