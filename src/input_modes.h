#ifndef TXMODE_INPUT_MODES_H
#define TXMODE_INPUT_MODES_H


typedef enum InputMode_e : int {
    IMODE_VIEW,
    IMODE_INSERT,
    IMODE_SELECT,
    //IMODE_REPLACE,


    IMODE_FILES,

    IMODE_COUNT
}
InputMode;



typedef struct Buffer_t Buffer;

typedef struct IModeCallbacks_t {
    void(*buffer_added)(Buffer* buf);
    void(*buffer_free)(Buffer* buf);
    void(*buffer_keypress)(Buffer* buf, int key, int mods);
    void(*buffer_chrpress)(Buffer* buf, char chr);
}
IModeCallbacks;

// Returns 'true' if cursor was moved.
bool imode_basic_cursor_movement       (Buffer* buf, int key, int mods);

extern void imode_INSERT_keypress      (Buffer* buf, int key, int mods);
extern void imode_INSERT_chrpress      (Buffer* buf, char c);

extern void imode_VIEW_keypress        (Buffer* buf, int key, int mods);
extern void imode_VIEW_chrpress        (Buffer* buf, char c);

extern void imode_SELECT_keypress      (Buffer* buf, int key, int mods);
extern void imode_SELECT_chrpress      (Buffer* buf, char c);

extern void imode_FILES_buffer_added   (Buffer* buf);
extern void imode_FILES_buffer_free    (Buffer* buf);
extern void imode_FILES_keypress       (Buffer* buf, int key, int mods);
extern void imode_FILES_chrpress       (Buffer* buf, char c);



#endif
