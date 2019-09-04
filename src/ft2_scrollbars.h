#ifndef __FT2_SCROLLBARS_H
#define __FT2_SCROLLBARS_H

#include <stdint.h>

enum /* SCROLLBARS */
{
    SB_POS_ED,
    SB_SAMPLE_LIST,
    SB_CHAN_SCROLL,
    SB_HELP_SCROLL,
    SB_SAMP_SCROLL,

    /* Instrument Editor */
    SB_INST_VOL,
    SB_INST_PAN,
    SB_INST_FTUNE,
    SB_INST_FADEOUT,
    SB_INST_VIBSPEED,
    SB_INST_VIBDEPTH,
    SB_INST_VIBSWEEP,

    /* Instrument Editor Extension */
    SB_INST_EXT_MIDI_CH,
    SB_INST_EXT_MIDI_PRG,
    SB_INST_EXT_MIDI_BEND,

    /* Config Audio */
    SB_AUDIO_OUTPUT_SCROLL,
    SB_AUDIO_INPUT_SCROLL,
    SB_AMP_SCROLL,
    SB_MASTERVOL_SCROLL,

    /* Config Layout */
    SB_PAL_R,
    SB_PAL_G,
    SB_PAL_B,
    SB_PAL_CONTRAST,

    /* Config Miscellaneous */
    SB_MIDI_SENS,

    /* Config Midi */
    SB_MIDI_INPUT_SCROLL,

    /* Disk Op. */
    SB_DISKOP_LIST,

    /* Sample volume box */
    SB_SAMPVOL_START,
    SB_SAMPVOL_END,

    /* Sample resample box */
    SB_RESAMPLE_HTONES,

    /* Sample mix sample box */
    SB_MIX_BALANCE,

    /* Sample echo box */
    SB_ECHO_NUM,
    SB_ECHO_DISTANCE,
    SB_ECHO_FADEOUT,

    NUM_SCROLLBARS
};

enum
{
    SCROLLBAR_UNPRESSED    = 0,
    SCROLLBAR_PRESSED      = 1,
    SCROLLBAR_HORIZONTAL   = 0,
    SCROLLBAR_VERTICAL     = 1,
    SCROLLBAR_THUMB_NOFLAT = 0,
    SCROLLBAR_THUMB_FLAT   = 1
};

typedef struct scrollBar_t /* DO NOT TOUCH!!! */
{
    uint16_t x, y, w, h;
    uint8_t visible, type, state;
    int64_t pos, page, end;
    uint16_t thumbX, thumbY, thumbW, thumbH;
    uint8_t thumbType;
    void (*callbackFunc)(int32_t pos);
} scrollBar_t;

void drawScrollBar(uint16_t scrollBarID);
void showScrollBar(uint16_t scrollBarID);
void hideScrollBar(uint16_t scrollBarID);
void scrollBarScrollUp(uint16_t scrollBarID, uint32_t amount);
void scrollBarScrollDown(uint16_t scrollBarID, uint32_t amount);
void setScrollBarPos(uint16_t scrollBarID, int64_t pos, int8_t triggerCallBack);
uint32_t getScrollBarPos(uint16_t scrollBarID);
void setScrollBarEnd(uint16_t scrollBarID, int64_t end);
void setScrollBarPageLength(uint16_t scrollBarID, int64_t pageLength);
int8_t testScrollBarMouseDown(void);
void testScrollBarMouseRelease(void);
void handleScrollBarsWhileMouseDown(void);
void updateScrollBarPalette(void);
void initializeScrollBars(void);

#endif
