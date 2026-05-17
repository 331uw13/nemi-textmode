#ifndef TXMODE_INPUT_MODES_H
#define TXMODE_INPUT_MODES_H


typedef enum InputMode_e {
    IMODE_VIEW,
    IMODE_INSERT,
    //IMODE_REPLACE,
    //IMODE_SELECT
}
InputMode;



// Shared input is always processed,
// It doesnt matter what the buffer's input mode is.
extern void handle_input_mode_shared_keypress  (int key, int mods);
extern void handle_input_mode_insert_keypress  (int key, int mods);
extern void handle_input_mode_insert_charpress (char c);
extern void handle_input_mode_view_keypress    (int key, int mods);
extern void handle_input_mode_view_charpress   (char c);


#endif
