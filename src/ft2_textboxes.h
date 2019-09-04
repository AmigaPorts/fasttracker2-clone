#ifndef __FT2_TEXTBOXES_H
#define __FT2_TEXTBOXES_H

#include <stdint.h>
#include <SDL2/SDL.h>

enum /* TEXTBOXES */
{
    TB_INST1,
    TB_INST2,
    TB_INST3,
    TB_INST4,
    TB_INST5,
    TB_INST6,
    TB_INST7,
    TB_INST8,

    TB_SAMP1,
    TB_SAMP2,
    TB_SAMP3,
    TB_SAMP4,
    TB_SAMP5,

    TB_SONG_NAME,

    TB_NIB_PLAYER1_NAME,
    TB_NIB_PLAYER2_NAME,

    TB_CONF_DEF_MODS_DIR,
    TB_CONF_DEF_INSTRS_DIR,
    TB_CONF_DEF_SAMPS_DIR,
    TB_CONF_DEF_PATTS_DIR,
    TB_CONF_DEF_TRACKS_DIR,

    TB_DISKOP_FILENAME,
    TB_DISKOP_RENAME_NAME,
    TB_DISKOP_MAKEDIR_NAME,
    TB_DISKOP_SETPATH_NAME,

    TB_SAVE_RANGE_FILENAME,

    TB_SCALE_FADE_VOL,

    NUM_TEXTBOXES
};

#define TEXT_CURSOR_BLINK_RATE 6
#define TEXT_SCROLL_VALUE 30

enum
{
    TEXTBOX_NO_UPDATE = 0,
    TEXTBOX_UPDATE    = 1
};

typedef struct textBox_t /* DO NOT TOUCH!!! */
{
    uint16_t x, y, w;
    uint8_t h, tx, ty;
    uint16_t tw, maxChars;
    uint8_t rightMouseButton;

    /* these ones are changed at run time */
    char *textPtr;
    uint8_t changeMouseCursor, visible, *renderBuf;
    int16_t cursorPos;
    uint16_t renderBufHeight;
    int32_t bufOffset, renderBufWidth;
} textBox_t;

int8_t textIsMarked(void);
void exitTextEditing(void);
int16_t getTextCursorX(textBox_t *t);
int16_t getTextCursorY(textBox_t *t);
void drawTextBox(uint16_t textBoxID);
void showTextBox(uint16_t textBoxID);
void hideTextBox(uint16_t textBoxID);
void setupTextBoxForSysReq(int16_t textBoxID, char *textPtr, int16_t textPtrLen, uint8_t setCursorToEnd);
int8_t testTextBoxMouseDown(void);
void updateTextBoxPointers(void);
void setupInitialTextBoxPointers(void);
void handleTextEditControl(SDL_Keycode keycode);
void handleTextEditInputChar(char textChar);
void handleTextBoxWhileMouseDown(void);
void freeTextBoxes(void);

#endif
