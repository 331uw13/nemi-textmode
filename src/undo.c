#include <assert.h>
#include <stdlib.h>

#include "undo.h"
#include "buffer.h"
#include "text_mode.h"
#include "nemi.h"

#define UBCOMMANDS_ALLOC_MORE 64


UndoStack undostack_init() {
    return (UndoStack) {
        .commands = NULL,
        .commands_count = 0,
        .commands_alloc = 0,
        .flags = 0
    };
}

void undostack_free(UndoStack* ub) {
    if(ub->commands != NULL) {
        free(ub->commands);
    }
}

bool undostack_isempty(UndoStack* ub) {
    return (ub->commands == NULL || ub->commands_count == 0);
}

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


void undostack_execute(UndoStackCmd cmd) {

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
            logprintf(LOG_INFO, "(undostack) CURSOR_MOVED");
            {
                buffer_move_cursor_to(buf,
                        cmd.location.row,
                        cmd.location.col);
            }
            break;

        case UCMD_INSERT_CHAR:
            logprintf(LOG_INFO, "(undostack) INSERT_CHAR");
            {
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_delete_char(row, cmd.location.col);
            }
            break;
        
        case UCMD_DELETE_CHAR:
            logprintf(LOG_INFO, "(undostack) DELETE_CHAR");
            {
                Bufrow* row = buffer_get_row(buf, cmd.location.row);
                bufrow_insert_char(row, cmd.location.col, cmd.data_as.character);
            }
            break;

        default:
            logprintf(LOG_WARN, "Executing undostack command but its not handled in switch statement.");
            break;
    }
    
    buf->undostack.flags &= ~UNDOSTACK_IGNORE_PUSH;
}

