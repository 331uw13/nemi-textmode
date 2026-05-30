#ifndef UNDO_BUFFER_H
#define UNDO_BUFFER_H

#include "types.h"
#include "buffer_id.h"


typedef enum UndoStackCmdKind_e {
    UCMD_CURSOR_MOVED,
    UCMD_INSERT_CHAR,
    UCMD_DELETE_CHAR,
    UCMD_INSERT_BLOCK,
    UCMD_DELETE_BLOCK,
    UCMD_INSERT_ROW,
    UCMD_DELETE_ROW,
}
UndoStackCmdKind;


typedef struct UndoStackCmdLocation_t {
    BufferID buf_id;
    ssize_t row;
    ssize_t col;
}
UndoStackCmdLocation;


typedef struct UndoStackCmd_t {
    UndoStackCmdKind kind;
    UndoStackCmdLocation location;
    
    union {
        char* char_array;
        char  character;
    }
    data_as;
    size_t data_len;
}
UndoStackCmd;


typedef struct UndoStackCmdGroup_t {
    UndoStackCmd*      commands;
    size_t             commands_count;
    size_t             commands_alloc;
}
UndoStackCmdGroup;

typedef struct UndoStack_t {
    UndoStackCmdGroup* cmd_groups;
    size_t             cmd_groups_count;
    size_t             cmd_groups_alloc;
    int flags;
}
UndoStack;


// If set, 'undostack_push()' dont have any effect.
// This is used for 'undostack_execute()' 
// to now push the actions back to the same undo stack.
#define UNDOSTACK_IGNORE_PUSH (1 << 0)



UndoStack  undostack_init();
void       undostack_free(UndoStack* ub);

void       undostack_start_new_group    (UndoStack* ub);
void       undostack_finish_group       (UndoStack* ub);
void       undostack_push               (UndoStack* ub, UndoStackCmd cmd);
void       undostack_pop_and_exec_group (UndoStack* ub);


#endif
