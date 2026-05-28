#include "undo.h"


UndoStack undobuffer_init() {
    return (UndoStack) {
        
    };
}

void undobuffer_free(UndoStack* ub) {
}

void undobuffer_push(UndoStack* ub, Ucmd cmd) {
}

UndoStack undobuffer_pop(UndoStack* ub) {
}


