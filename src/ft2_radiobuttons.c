/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_config.h"
#include "ft2_help.h"
#include "ft2_sample_ed.h"
#include "ft2_nibbles.h"
#include "ft2_inst_ed.h"
#include "ft2_diskop.h"
#include "ft2_mouse.h"
#include "ft2_wav_renderer.h"

radioButton_t radioButtons[NUM_RADIOBUTTONS] =
{
    // ------ HELP SCREEN RADIOBUTTONS ------
    //x,   y,   w,  initial state,         visible, group,         funcOnUp
    {   5, 16,  68, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpFeatures },
    {   5, 31,  59, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpEffects },
    {   5, 46,  70, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpKeyboard },
    {   5, 61, 108, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpHowToUseFT2 },
    {   5, 76, 100, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpFAQ },
    {   5, 91,  85, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_HELP, rbHelpKnownBugs },

    // ------ NIBBLES SCREEN RADIOBUTTONS ------
    //x,   y,   w,  initial state,         visible, group,                       funcOnUp
    {  4, 105,  62, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_PLAYERS,    nibblesSet1Player },
    {  4, 119,  69, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_PLAYERS,    nibblesSet2Players },
    { 79, 117,  55, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_DIFFICULTY, nibblesSetNovice },
    { 79, 131,  63, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_DIFFICULTY, nibblesSetAverage },
    { 79, 145,  34, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_DIFFICULTY, nibblesSetPro },
    { 79, 159,  50, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_NIBBLES_DIFFICULTY, nibblesSetTriton },

    // ------ SAMPLER SCREEN RADIOBUTTONS ------
    //x,   y,   w,   initial state,         visible, group,                 funcOnUp
    { 357, 351,  58, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_SAMPLE_LOOP,  rbSampleNoLoop },
    { 357, 368,  62, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_SAMPLE_LOOP,  rbSampleForwardLoop },
    { 357, 385,  67, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_SAMPLE_LOOP,  rbSamplePingpongLoop },
    { 431, 368,  44, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_SAMPLE_DEPTH, rbSample8bit },
    { 431, 383,  50, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_SAMPLE_DEPTH, rbSample16bit },

    // ------ INSTRUMENT EDITOR SCREEN RADIOBUTTONS ------
    //x,   y,   w,   initial state,         visible, group,                  funcOnUp
    { 442, 279,  25, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_INST_WAVEFORM, rbVibWaveSine },
    { 472, 279,  25, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_INST_WAVEFORM, rbVibWaveSquare },
    { 502, 279,  25, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_INST_WAVEFORM, rbVibWaveRampDown },
    { 532, 279,  25, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_INST_WAVEFORM, rbVibWaveRampUp },

    // ------ CONFIG SCREEN LEFT RADIOBUTTONS ------
    //x,   y,   w,   initial state,         visible, group,                  funcOnUp
    {   4,  19,  88, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SELECT, rbConfigIODevices },
    {   4,  35,  59, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SELECT, rbConfigLayout },
    {   4,  51,  99, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SELECT, rbConfigMiscellaneous },
    {   4,  67,  74, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SELECT, rbConfigMidiInput },

    // ------ CONFIG AUDIO ------

    // audio buffer size
    //x,   y,   w,   initial state,         visible, group,                           funcOnUp
    { 390,  16,  46, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SOUND_BUFF_SIZE, rbConfigSbs512  },
    { 390,  30, 113, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SOUND_BUFF_SIZE, rbConfigSbs1024 },
    { 390,  44,  50, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SOUND_BUFF_SIZE, rbConfigSbs2048 },
    { 390,  58,  76, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SOUND_BUFF_SIZE, rbConfigSbs4096 },

    // audio bit depth
    //x,   y,   w,   initial state,         visible, group,                           funcOnUp
    { 390,  89, 107, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_BIT_DEPTH, rbConfigAudio16bit },
    { 390, 103,  83, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_BIT_DEPTH, rbConfigAudio24bit },

    // audio output frequency
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    { 509,  16,  66, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio32kHz },
#ifdef __APPLE__
    { 509,  30, 121, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio44kHz },
    { 509,  44,  66, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio48kHz },
#else
    { 509,  30,  66, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio44kHz },
    { 509,  44, 121, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio48kHz },
#endif
    { 509,  58,  66, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_AUDIO_FREQ, rbConfigAudio96kHz },

    // frequency table
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    { 509,  89, 117, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FREQ_TABLE, rbConfigFreqTableAmiga  },
    { 509, 103, 120, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FREQ_TABLE, rbConfigFreqTableLinear },

    // ------ CONFIG LAYOUT ------

    // mouse shape
    //x,   y,   w,   initial state,         visible, group,                 funcOnUp
    { 115, 120,  41, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE, rbConfigMouseNice },
    { 178, 120,  41, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE, rbConfigMouseUgly },
    { 115, 134,  47, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE, rbConfigMouseAwful },
    { 178, 134,  62, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE, rbConfigMouseUseable },

    // mouse busy shape
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    { 115, 159,  51, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE_BUSY, rbConfigMouseBusyVogue },
    { 178, 159,  45, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_MOUSE_BUSY, rbConfigMouseBusyMrH },

    // scope style
    //x,   y,   w,   initial state,         visible, group,                 funcOnUp
    { 257, 158,  58, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SCOPE, rbConfigScopeOriginal },
    { 323, 158,  47, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_SCOPE, rbConfigScopeLined },

    // visible pattern channels
    //x,   y,   w,   initial state,         visible, group,                         funcOnUp
    { 257,  42,  78, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PATTERN_CHANS, rbConfigPatt4Chans },
    { 257,  56,  78, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PATTERN_CHANS, rbConfigPatt6Chans },
    { 257,  70,  78, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PATTERN_CHANS, rbConfigPatt8Chans },
    { 257,  84,  85, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PATTERN_CHANS, rbConfigPatt12Chans },

    // pattern font
    //x,   y,   w,   initial state,         visible, group,                funcOnUp
    { 257, 114,  62, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FONT, rbConfigFontCapitals },
    { 323, 114,  68, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FONT, rbConfigFontLowerCase },
    { 257, 129,  54, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FONT, rbConfigFontFuture },
    { 323, 129,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FONT, rbConfigFontBold },

    // palette entries
    //x,   y,   w,   initial state,         visible, group,                       funcOnUp
    { 399,   2,  88, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalPatternText },
    { 399,  16,  79, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalBlockMark },
    { 399,  30,  97, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalTextOnBlock },
    { 399,  44,  52, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalMouse },
    { 399,  58,  63, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalDesktop },
    { 399,  72,  61, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_ENTRIES, rbConfigPalButttons },

    // palette presets
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    { 399,  89,  50, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalArctic },
    { 512,  89,  81, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalLitheDark },
    { 399, 103, 105, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalAuroraBorealis },
    { 512, 103,  45, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalRose },
    { 399, 117,  47, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalBlues },
    { 512, 117,  81, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalSpacePigs },
    { 399, 131,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalGold },
    { 512, 131,  56, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalViolent },
    { 399, 145,  87, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalHeavyMetal },
    { 512, 145,  91, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalWhyColors },
    { 399, 159,  54, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalJungle },
    { 512, 159,  90, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_PAL_PRESET, rbConfigPalUserDefined },

    // ------ CONFIG MISCELLANEOUS ------

    // FILENAME SORTING
    //x,   y,   w,   initial state,         visible, group,                    funcOnUp
    { 114,  15,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FILESORT, rbFileSortExt },
    { 114,  29,  48, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_FILESORT, rbFileSortName },

    // WINDOW SIZE
    //x,   y,   w,   initial state,         visible, group,                    funcOnUp
    { 114,  58,  60, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_WIN_SIZE, rbWinSizeAuto },
    { 114,  72,  31, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_WIN_SIZE, rbWinSize1x },
    { 156,  72,  31, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_WIN_SIZE, rbWinSize3x },
    { 114,  86,  31, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_WIN_SIZE, rbWinSize2x },
    { 156,  86,  31, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_CONFIG_WIN_SIZE, rbWinSize4x },

    // ------ DISK OP. ------

    // FILENAME SORTING
    //x,   y,   w,   initial state,         visible, group,                funcOnUp
    {   4,  16,  55, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_ITEM, rbDiskOpModule },
    {   4,  30,  45, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_ITEM, rbDiskOpInstr },
    {   4,  44,  56, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_ITEM, rbDiskOpSample },
    {   4,  58,  59, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_ITEM, rbDiskOpPattern },
    {   4,  72,  50, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_ITEM, rbDiskOpTrack },

    // MODULE SAVE AS FORMATS
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    {   4, 100,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_MOD_SAVEAS, rbDiskOpModSaveMod },
    {   4, 114,  33, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_MOD_SAVEAS, rbDiskOpModSaveXm },
    {   4, 128,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_MOD_SAVEAS, rbDiskOpModSaveWav },

    // INSTRUMENT SAVE AS FORMATS
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    {   4, 100,  29, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_INS_SAVEAS, NULL },

    // SAMPLE SAVE AS FORMATS
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    {   4, 100,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_SMP_SAVEAS, rbDiskOpSmpSaveRaw },
    {   4, 114,  34, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_SMP_SAVEAS, rbDiskOpSmpSaveIff },
    {   4, 128,  40, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_SMP_SAVEAS, rbDiskOpSmpSaveWav },

    // PATTERN SAVE AS FORMATS
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    {   4, 100,  33, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_PAT_SAVEAS, NULL },

    // TRACK SAVE AS FORMATS
    //x,   y,   w,   initial state,         visible, group,                      funcOnUp
    {   4, 100,  31, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_DISKOP_TRK_SAVEAS, NULL },

    // WAV RENDERER BITDEPTH
    //x,   y,   w,   initial state,         visible, group,                        funcOnUp
    { 140, 95,   52, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_WAV_RENDER_BITDEPTH, rbWavRenderBitDepth16 },
    { 205, 95,   83, RADIOBUTTON_UNCHECKED, false,   RB_GROUP_WAV_RENDER_BITDEPTH, rbWavRenderBitDepth32 },
};

/* defined at the bottom of this file */
extern const uint8_t radioButtonGraphics[3][RADIOBUTTON_W * RADIOBUTTON_H];

void drawRadioButton(uint16_t radioButtonID)
{
    uint8_t state;
    radioButton_t *radioButton;

    MY_ASSERT(radioButtonID < NUM_RADIOBUTTONS)

    radioButton = &radioButtons[radioButtonID];
    if (!radioButton->visible)
        return;

    MY_ASSERT((radioButton->x < SCREEN_W) && (radioButton->y < SCREEN_H))

    state = radioButton->state;
    if (state <= (RADIOBUTTON_STATES - 1))
        blitFast(radioButton->x, radioButton->y, radioButtonGraphics[state], RADIOBUTTON_W, RADIOBUTTON_H);
}

void showRadioButton(uint16_t radioButtonID)
{
    MY_ASSERT(radioButtonID < NUM_RADIOBUTTONS)

    radioButtons[radioButtonID].visible = true;
    drawRadioButton(radioButtonID);
}

void hideRadioButton(uint16_t radioButtonID)
{
    MY_ASSERT(radioButtonID < NUM_RADIOBUTTONS)

    radioButtons[radioButtonID].visible = false;
}

void checkRadioButton(uint16_t radioButtonID)
{
    uint16_t i;
    radioButton_t *radioButton;

    MY_ASSERT(radioButtonID < NUM_RADIOBUTTONS)

    radioButton = &radioButtons[radioButtonID];

    for (i = 0; i < NUM_RADIOBUTTONS; ++i)
    {
        if (radioButtons[i].group == radioButton->group)
        {
            if (radioButtons[i].state == RADIOBUTTON_CHECKED)
            {
                radioButtons[i].state = RADIOBUTTON_UNCHECKED;
                drawRadioButton(i);
                break;
            }
        }
    }

    MY_ASSERT(i < NUM_RADIOBUTTONS)

    radioButtons[radioButtonID].state = RADIOBUTTON_CHECKED;
    drawRadioButton(radioButtonID);
}

void uncheckRadioButtonGroup(uint16_t radioButtonGroup)
{
    uint16_t i;

    for (i = 0; i < NUM_RADIOBUTTONS; ++i)
    {
        if (radioButtons[i].group == radioButtonGroup)
            radioButtons[i].state = RADIOBUTTON_UNCHECKED;
    }
}

void showRadioButtonGroup(uint16_t radioButtonGroup)
{
    uint16_t i;

    for (i = 0; i < NUM_RADIOBUTTONS; ++i)
    {
        if (radioButtons[i].group == radioButtonGroup)
            showRadioButton(i);
    }
}

void hideRadioButtonGroup(uint16_t radioButtonGroup)
{
    uint16_t i;

    for (i = 0; i < NUM_RADIOBUTTONS; ++i)
    {
        if (radioButtons[i].group == radioButtonGroup)
            hideRadioButton(i);
    }
}

void handleRadioButtonsWhileMouseDown(void)
{
    radioButton_t *radioButton;

    MY_ASSERT((mouse.lastUsedObjectID >= 0) && (mouse.lastUsedObjectID < NUM_RADIOBUTTONS))

    radioButton = &radioButtons[mouse.lastUsedObjectID];
    if (!radioButton->visible)
        return;

    if (radioButton->state != RADIOBUTTON_CHECKED)
    {
        radioButton->state = RADIOBUTTON_UNCHECKED;
        if ((mouse.x >= radioButton->x) && (mouse.x < (radioButton->x + radioButton->clickAreaWidth)))
        {
            if ((mouse.y >= radioButton->y) && (mouse.y < (radioButton->y + RADIOBUTTON_H)))
                radioButton->state = RADIOBUTTON_PRESSED;
        }

        if ((mouse.lastX != mouse.x) || (mouse.lastY != mouse.y))
        {
            mouse.lastX = mouse.x;
            mouse.lastY = mouse.y;

            drawRadioButton(mouse.lastUsedObjectID);
        }
    }
}

int8_t testRadioButtonMouseDown(void)
{
    uint16_t i;
    radioButton_t *radioButton;

    for (i = 0; i < NUM_RADIOBUTTONS; ++i)
    {
        radioButton = &radioButtons[i];
        if (radioButton->visible && (radioButton->state != RADIOBUTTON_CHECKED))
        {
            if ((mouse.x >= radioButton->x) && (mouse.x < (radioButton->x + radioButton->clickAreaWidth)))
            {
                if ((mouse.y >= radioButton->y) && (mouse.y < (radioButton->y + RADIOBUTTON_H)))
                {
                    mouse.lastUsedObjectID   = i;
                    mouse.lastUsedObjectType = OBJECT_RADIOBUTTON;

                    return (true);
                }
            }
        }
    }

    return (false);
}

void testRadioButtonMouseRelease(void)
{
    radioButton_t *radioButton;

    if (mouse.lastUsedObjectType == OBJECT_RADIOBUTTON)
    {
        if (mouse.lastUsedObjectID != OBJECT_ID_NONE)
        {
            MY_ASSERT(mouse.lastUsedObjectID < NUM_RADIOBUTTONS)

            radioButton = &radioButtons[mouse.lastUsedObjectID];
            if (radioButton->visible && (radioButton->state != RADIOBUTTON_CHECKED))
            {
                if ((mouse.x >= radioButton->x) && (mouse.x < (radioButton->x + radioButton->clickAreaWidth)))
                {
                    if ((mouse.y >= radioButton->y) && (mouse.y < (radioButton->y + RADIOBUTTON_H + 2)))
                    {
                        radioButton->state = RADIOBUTTON_UNCHECKED;
                        drawRadioButton(mouse.lastUsedObjectID);

                        if (radioButton->callbackFunc != NULL)
                           (radioButton->callbackFunc)();
                    }
                }
            }
        }
    }
}

const uint8_t radioButtonGraphics[3][RADIOBUTTON_W * RADIOBUTTON_H] =
{
    /* unticked */
    {
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP1, PAL_DSKTOP2, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP2, PAL_DSKTOP1,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DSKTOP2, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2, PAL_DSKTOP1, PAL_DESKTOP,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_BCKGRND,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_BCKGRND, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_BCKGRND,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_BCKGRND,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2, PAL_BCKGRND,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DSKTOP2, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP
    },

    /* ticked */
    {
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_BCKGRND, PAL_DSKTOP2,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_FORGRND, PAL_DESKTOP,
        PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_FORGRND, PAL_FORGRND,
        PAL_FORGRND, PAL_DESKTOP, PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_FORGRND, PAL_FORGRND,
        PAL_FORGRND, PAL_FORGRND, PAL_FORGRND, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP,
        PAL_FORGRND, PAL_FORGRND, PAL_FORGRND, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_FORGRND, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP
    },

    /* pressed */
    {
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_BCKGRND, PAL_DSKTOP2,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_BCKGRND, PAL_DSKTOP2, PAL_DESKTOP,
        PAL_DSKTOP2, PAL_BCKGRND, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1,
        PAL_DSKTOP2, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2, PAL_DESKTOP, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DSKTOP1, PAL_DSKTOP2,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DSKTOP1, PAL_DSKTOP1, PAL_DESKTOP, PAL_DESKTOP, PAL_DESKTOP,
        PAL_DESKTOP
    }
};
