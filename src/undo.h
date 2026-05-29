#ifndef UNDO_BUFFER_H
#define UNDO_BUFFER_H

#include "types.h"
#include "buffer_id.h"


typedef enum UndoStackCmdKind_e {
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
}
UndoStack;



UndoStack  undobuffer_init();
void       undobuffer_free(UndoStack* ub);

void       undobuffer_push(UndoStack* ub, UndoStackCmd cmd);
UndoStackCmd  undobuffer_pop(UndoStack* ub);



#endif
