#ifndef UNDO_BUFFER_H
#define UNDO_BUFFER_H



typedef Buffer_t Buffer;


typedef enum UStackCmdKind_e {
    UCMD_INSERT_CHAR,
    UCMD_DELETE_CHAR,
    UCMD_INSERT_ROW,
    UCMD_DELETE_ROW,
}
UStackCmdKind;


typedef struct UStackCmdLocation_t {
    Buffer* buf;
    ssize_t row;
    ssize_t col;
}
UStackCmdLocation;


typedef struct UStackCmd_t {
    UStackCmdKind kind;
    UStackCmdLocation location;
    
    union {
        char* char_array;
        char  character;
    }
    data_as;
}
UStackCmd;


typedef struct UndoStack_t {
    UStackCmd* commands;
    size_t commands_count;
    size_t commands_alloc;
}
UndoStack;



UndoStack  undobuffer_init();
void       undobuffer_free(UndoStack* ub);

void       undobuffer_push(UndoStack* ub, UStackCmd cmd);
UStackCmd  undobuffer_pop(UndoStack* ub);



#endif
