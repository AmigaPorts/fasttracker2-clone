/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_config.h"
#include "ft2_sample_ed.h"
#include "ft2_nibbles.h"
#include "ft2_inst_ed.h"
#include "ft2_pattern_ed.h"
#include "ft2_trim.h"
#include "ft2_mouse.h"
#include "ft2_edit.h"

checkBox_t checkBoxes[NUM_CHECKBOXES] =
{
    // ------ SYSTEM REQUEST CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   0, 299, 116,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   configToggleS3MLoadWarning },
    {   0, 299, 116,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   configToggleNotYetAppliedWarning },
    { 176, 268, 146,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbEchoAddMemory },

    // ------ NIBBLES CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   3, 133,  70,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   nibblesToggleSurround },
    {   3, 146,  40,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   nibblesToggleGrid },
    {   3, 159,  45,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   nibblesToggleWrap },

    // ------ ADVANCED EDIT CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    { 113,  94, 105,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMaskEnable },
    { 237, 107,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMask0 },
    { 237, 120,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMask1 },
    { 237, 133,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMask2 },
    { 237, 146,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMask3 },
    { 237, 159,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleCopyMask4 },
    { 256, 107,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   togglePasteMask0 },
    { 256, 120,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   togglePasteMask1 },
    { 256, 133,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   togglePasteMask2 },
    { 256, 146,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   togglePasteMask3 },
    { 256, 159,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   togglePasteMask4 },
    { 275, 107,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleTranspMask0 },
    { 275, 120,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleTranspMask1 },
    { 275, 133,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleTranspMask2 },
    { 275, 146,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleTranspMask3 },
    { 275, 159,  13,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   toggleTranspMask4 },

    // ------ INSTRUMENT EDITOR CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   3, 175, 118,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbVEnv },
    { 341, 192,  64,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbVEnvSus },
    { 341, 217,  70,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbVEnvLoop },
    {   3, 262, 123,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbPEnv },
    { 341, 279,  64,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbPEnvSus },
    { 341, 304,  70,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbPEnvLoop },

    // ------ INSTRUMENT EDITOR EXTENSION CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   3, 112, 148,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbInstMidiEnable },
    { 172, 112, 103,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbInstMuteComputer },

    // ------ TRIM SCREEN CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   3, 107, 113,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimUnusedPatt    },
    {   3, 120, 132,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimUnusedInst    },
    {   3, 133, 110,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimUnusedSamp    },
    {   3, 146, 115,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimUnusedChans   },
    {   3, 159, 130,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimUnusedSmpData },
    { 139,  94, 149,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbTrimSmpsTo8Bit    },

    // ------ CONFIG CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    {   3,  91,  76,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbToggleAutoSaveConfig },

    // ------ CONFIG AUDIO CHECKBOXES ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    { 389, 132,  90,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigInterpolation },
    { 389, 145, 107,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigVolRamp },
    { 389, 158 , 94,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigDither },

    // ------ CONFIG LAYOUT CHECKBOXES  ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    { 113,  14, 108,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigPattStretch },
    { 113,  27,  77,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigHexCount },
    { 113,  40,  81,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigAccidential },
    { 113,  53,  92,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigShowZeroes },
    { 113,  66,  81,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigFramework },
    { 113,  79, 128,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigLineColors },
    { 113,  92, 126,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigChanNums },
    { 255,  14, 136,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbConfigShowVolCol },

    // ------ CONFIG MISCELLANEOUS CHECKBOXES  ------
    //x,   y,   w,   h,   initial state,      checked,            visible, funcOnUp
    { 212,   2, 142,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbSampCutToBuff },
    { 212,  15, 145,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbPattCutToBuff },
    { 212,  28, 153,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbKillNotesAtStop },
    { 212,  41, 149,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbFileOverwriteWarn },

    { 212,  69, 130,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMultiChanRec },
    { 212,  82, 153,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMultiChanKeyJazz },
    { 212,  95, 114,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMultiChanEdit },

    { 212, 108, 162,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbRecKeyOff },
    { 212, 121,  77,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbQuantisize },
    { 212, 134, 180,  25, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbChangePattLenInsDel },
    { 212, 159, 187,  25, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIAllowPC },

    { 411,  93,  83,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIEnable },
    { 530, 106,  30,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIRecAllChn },
    { 411, 119, 121,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIRecTransp },
    { 411, 132, 109,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIRecVelosity },
    { 411, 145, 124,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbMIDIRecAftert },

    { 113, 115,  74,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbVsyncOff },
    { 113, 128,  78,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbFullScreen },
    { 113, 141,  78,  12, CHECKBOX_UNPRESSED, CHECKBOX_UNCHECKED, false,   cbPixelFilter }
};

/* these are defined at the bottom of this file */
extern const uint8_t checkMarkGraphics[CHECKMARK_W * CHECKMARK_H];
extern const uint8_t accidentalFlat[5 * 6];
extern const uint8_t accidentalSharp[5 * 6];

void drawCheckBox(uint16_t checkBoxID)
{
    uint8_t state;
    uint16_t x, y, w, h;
    checkBox_t *checkBox;
    const uint8_t *srcPtr;

    MY_ASSERT(checkBoxID < NUM_CHECKBOXES)

    checkBox = &checkBoxes[checkBoxID];
    if (!checkBox->visible)
        return;

    state = checkBox->state;

    x = checkBox->x;
    y = checkBox->y;
    w = CHECKBOX_W;
    h = CHECKBOX_H;

    /* fill button background */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* draw outer border */
    hLine(x,         y,         w, PAL_BCKGRND);
    hLine(x,         y + h - 1, w, PAL_BCKGRND);
    vLine(x,         y,         h, PAL_BCKGRND);
    vLine(x + w - 1, y,         h, PAL_BCKGRND);

    /* draw inner borders */
    if (state == CHECKBOX_UNPRESSED)
    {
        /* top left corner inner border */
        hLine(x + 1, y + 1, w - 3, PAL_BUTTON1);
        vLine(x + 1, y + 2, h - 4, PAL_BUTTON1);

        /* bottom right corner inner border */
        hLine(x + 1 - 0, y + h - 2, w - 2, PAL_BUTTON2);
        vLine(x + w - 2, y + 1 - 0, h - 3, PAL_BUTTON2);
    }
    else
    {
        /* top left corner inner border */
        hLine(x + 1, y + 1, w - 2, PAL_BUTTON2);
        vLine(x + 1, y + 2, h - 3, PAL_BUTTON2);
    }

    /* for the special "Accidental" check button in Config Layout */
    srcPtr = (config.ptnAcc == 1) ? accidentalSharp : accidentalFlat;

    /* draw check mark (if checked) */
    if (checkBox->checked)
    {
        if (checkBoxID == CB_CONF_ACCIDENTAL)
        {
            if (state == CHECKBOX_PRESSED)
                blitFast(x + 5, y + 4, srcPtr, 5, 6);
            else
                blitFast(x + 4, y + 3, srcPtr, 5, 6);
        }
        else
        {
            if (state == CHECKBOX_PRESSED)
                blitFast(x + 3, y + 4, checkMarkGraphics, CHECKMARK_W, CHECKMARK_H);
            else
                blitFast(x + 2, y + 3, checkMarkGraphics, CHECKMARK_W, CHECKMARK_H);
        }
    }
    else
    {
        if (checkBoxID == CB_CONF_ACCIDENTAL)
        {
            if (state == CHECKBOX_PRESSED)
                blitFast(x + 5, y + 4, srcPtr, 5, 6);
            else
                blitFast(x + 4, y + 3, srcPtr, 5, 6);
        }
    }
}

void showCheckBox(uint16_t checkBoxID)
{
    MY_ASSERT(checkBoxID < NUM_CHECKBOXES)

    checkBoxes[checkBoxID].visible = true;
    drawCheckBox(checkBoxID);
}

void hideCheckBox(uint16_t checkBoxID)
{
    MY_ASSERT(checkBoxID < NUM_CHECKBOXES)

    checkBoxes[checkBoxID].visible = false;
}

void handleCheckBoxesWhileMouseDown(void)
{
    checkBox_t *checkBox;

    MY_ASSERT((mouse.lastUsedObjectID >= 0) && (mouse.lastUsedObjectID < NUM_CHECKBOXES))

    checkBox = &checkBoxes[mouse.lastUsedObjectID];
    if (!checkBox->visible)
        return;

    checkBox->state = CHECKBOX_UNPRESSED;
    if ((mouse.x >= checkBox->x) && (mouse.x < (checkBox->x + checkBox->clickAreaWidth)))
    {
        if ((mouse.y >= checkBox->y) && (mouse.y < (checkBox->y + checkBox->clickAreaHeight)))
            checkBox->state = CHECKBOX_PRESSED;
    }

    if ((mouse.lastX != mouse.x) || (mouse.lastY != mouse.y))
    {
        mouse.lastX = mouse.x;
        mouse.lastY = mouse.y;

        drawCheckBox(mouse.lastUsedObjectID);
    }
}

int8_t testCheckBoxMouseDown(void)
{
    uint16_t i;
    checkBox_t *checkBox;

    for (i = 0; i < NUM_CHECKBOXES; ++i)
    {
        checkBox = &checkBoxes[i];
        if (checkBox->visible)
        {
            /* if sys req. is shown, only allow certain checkboxes */
            if (editor.ui.systemRequestShown)
            {
                switch (i)
                {
                    default: continue;

                    case CB_SR_S3M_DONT_SHOW:
                    case CB_SR_NOT_YET_APPLIED_DONT_SHOW:
                    case CB_SAMP_ECHO_ADD_MEMORY:
                        break;
                }
            }

            if ((mouse.x >= checkBox->x) && (mouse.x < (checkBox->x + checkBox->clickAreaWidth)))
            {
                if ((mouse.y >= checkBox->y) && (mouse.y < (checkBox->y + checkBox->clickAreaHeight)))
                {
                    mouse.lastUsedObjectID   = i;
                    mouse.lastUsedObjectType = OBJECT_CHECKBOX;

                    checkBox->state = CHECKBOX_PRESSED;
                    drawCheckBox(mouse.lastUsedObjectID);

                    return (true);
                }
            }
        }
    }

    return (false);
}

void testCheckBoxMouseRelease(void)
{
    checkBox_t *checkBox;

    if (mouse.lastUsedObjectType == OBJECT_CHECKBOX)
    {
        if (mouse.lastUsedObjectID != OBJECT_ID_NONE)
        {
            MY_ASSERT(mouse.lastUsedObjectID < NUM_CHECKBOXES)

            checkBox = &checkBoxes[mouse.lastUsedObjectID];
            if (checkBox->visible)
            {
                if ((mouse.x >= checkBox->x) && (mouse.x < (checkBox->x + checkBox->clickAreaWidth)))
                {
                    if ((mouse.y >= checkBox->y) && (mouse.y < (checkBox->y + checkBox->clickAreaHeight)))
                    {
                        checkBox->checked ^= 1;

                        checkBox->state = CHECKBOX_UNPRESSED;
                        drawCheckBox(mouse.lastUsedObjectID);

                        if (checkBox->callbackFunc != NULL)
                           (checkBox->callbackFunc)();
                    }
                }
            }
        }
    }
}

const uint8_t checkMarkGraphics[CHECKMARK_W * CHECKMARK_H] =
{
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND,
    PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS
};

const uint8_t accidentalFlat[5 * 6] =
{
    PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND,
    PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND,
    PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BUTTONS
};

const uint8_t accidentalSharp[5 * 6] =
{
    PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BUTTONS, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS, PAL_BCKGRND, PAL_BCKGRND,
    PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BCKGRND, PAL_BUTTONS
};
