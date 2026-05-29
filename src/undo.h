#ifndef UNDO_BUFFER_H
#define UNDO_BUFFER_H

#include "types.h"
#include "buffer_id.h"


typedef enum UndoStackCmdKind_e {
    UCMD_CURSOR_MOVED,
    UCMD_INSERT_CHAR,
    UCMD_DELETE_CHAR,
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
}
UndoStackCmd;


typedef struct UndoStack_t {
    UndoStackCmd* commands;
    size_t commands_count;
    size_t commands_alloc;
    int    flags;
}
UndoStack;


// If set, 'undostack_push()' dont have any effect.
// This is used for 'undostack_execute()' 
// to now push the actions back to the same undo stack.
#define UNDOSTACK_IGNORE_PUSH (1 << 0)



UndoStack  undostack_init();
void       undostack_free(UndoStack* ub);

bool          undostack_isempty(UndoStack* ub);
void          undostack_push(UndoStack* ub, UndoStackCmd cmd);
UndoStackCmd  undostack_pop(UndoStack* ub);

void          undostack_execute(UndoStackCmd cmd);


#endif
