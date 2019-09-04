/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_about.h"
#include "ft2_video.h"
#include "ft2_config.h"
#include "ft2_audioselector.h"
#include "ft2_audio.h"
#include "ft2_help.h"
#include "ft2_sample_ed.h"
#include "ft2_nibbles.h"
#include "ft2_inst_ed.h"
#include "ft2_pattern_ed.h"
#include "ft2_sample_loader.h"
#include "ft2_diskop.h"
#include "ft2_wav_renderer.h"
#include "ft2_trim.h"
#include "ft2_sampling.h"
#include "ft2_module_loader.h"
#include "ft2_midi.h"
#include "ft2_midi.h"
#include "ft2_mouse.h"
#include "ft2_edit.h"

pushButton_t pushButtons[NUM_PUSHBUTTONS] =
{
    // ------ POSITION EDITOR PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,      line #2, funcOnDown,      funcOnUp
    {  55,   2,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbPosEdPosDown,  NULL,    false, NULL, NULL, true },
    {  55,  36,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbPosEdPosUp,    NULL,    false, NULL, NULL, true },
    {  74,   2,  35,  16, false,   PUSHBUTTON_UNPRESSED, 6, "Ins.",            NULL,    pbPosEdIns,      NULL,    false, NULL, NULL, true },
    {  74,  19,  18,  13, false,   PUSHBUTTON_UNPRESSED, 6, ARROW_UP_STRING,   NULL,    pbPosEdPattUp,   NULL,    false, NULL, NULL, true },
    {  91,  19,  18,  13, false,   PUSHBUTTON_UNPRESSED, 6, ARROW_DOWN_STRING, NULL,    pbPosEdPattDown, NULL,    false, NULL, NULL, true },
    {  74,  33,  35,  16, false,   PUSHBUTTON_UNPRESSED, 6, "Del.",            NULL,    pbPosEdDel,      NULL,    false, NULL, NULL, true },
    {  74,  50,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbPosEdLenUp,    NULL,    false, NULL, NULL, true },
    {  91,  50,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbPosEdLenDown,  NULL,    false, NULL, NULL, true },
    {  74,  62,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbPosEdRepSUp,   NULL,    false, NULL, NULL, true },
    {  91,  62,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbPosEdRepSDown, NULL,    false, NULL, NULL, true },

    // ------ SONG/PATTERN PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,      line #2, funcOnDown,     funcOnUp
    { 168,  34,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbBPMUp,        NULL,            false, NULL, NULL, true },
    { 185,  34,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbBPMDown,      NULL,            false, NULL, NULL, true },
    { 168,  48,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbSpeedUp,      NULL,            false, NULL, NULL, true },
    { 185,  48,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbSpeedDown,    NULL,            false, NULL, NULL, true },
    { 168,  62,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbEditSkipUp,   NULL,            false, NULL, NULL, true },
    { 185,  62,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbEditSkipDown, NULL,            false, NULL, NULL, true },
    { 253,  34,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbEditPattUp,   NULL,            false, NULL, NULL, true },
    { 270,  34,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbEditPattDown, NULL,            false, NULL, NULL, true },
    { 253,  48,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbPattLenUp,    NULL,            false, NULL, NULL, true },
    { 270,  48,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbPattLenDown,  NULL,            false, NULL, NULL, true },
    { 209,  62,  40,  13, false,   PUSHBUTTON_UNPRESSED, 3, "Expd.",           NULL,    NULL,           expandPattern,   false, NULL, NULL, true },
    { 248,  62,  40,  13, false,   PUSHBUTTON_UNPRESSED, 3, "Srnk.",           NULL,    NULL,           pbShrinkPattern, false, NULL, NULL, true },

    // ------ LOGO PUSHBUTTON ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    { 112,   0, 154,  32, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbToggleLogo,  false, NULL, NULL, true },
    { 266,   0,  25,  32, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbToggleBadge, false, NULL, NULL, true },

    // ------ MAIN MENU PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                  text line #1, line #2, funcOnDown, funcOnUp
    { 294,   2,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "About",      NULL,    NULL,       showAboutScreen,            false, NULL, NULL, true },
    { 294,  19,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Nibbles",    NULL,    NULL,       pbNibbles,                  false, NULL, NULL, true },
    { 294,  36,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Zap",        NULL,    NULL,       pbZap,                      false, NULL, NULL, true },
    { 294,  53,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Trim",       NULL,    NULL,       toggleTrimScreen,           false, NULL, NULL, true },
    { 294,  70,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Extend",     NULL,    NULL,       patternEditorExtended,      false, NULL, NULL, true },
    { 294,  87,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Transps.",   NULL,    NULL,       toggleTranspose,            false, NULL, NULL, true },
    { 294, 104,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "I.E.Ext.",   NULL,    NULL,       toggleInstEditorExt,        false, NULL, NULL, true },
    { 294, 121,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "S.E.Ext.",   NULL,    NULL,       toggleSampleEditorExt,      false, NULL, NULL, true },
    { 294, 138,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Adv. Edit",  NULL,    NULL,       toggleAdvEdit,              false, NULL, NULL, true },
    { 294, 155,  30,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Add",        NULL,    NULL,       pbAddChan,                  false, NULL, NULL, true },
    { 323, 155,  30,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Sub",        NULL,    NULL,       pbSubChan,                  false, NULL, NULL, true },
    { 359,   2,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Play sng.",  NULL,    NULL,       playSong,                   false, NULL, NULL, true },
    { 359,  19,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Play ptn.",  NULL,    NULL,       playPattern,                false, NULL, NULL, true },
    { 359,  36,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Stop",       NULL,    NULL,       stopPlaying,                false, NULL, NULL, true },
    { 359,  53,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Rec. sng.",  NULL,    NULL,       recordSong,                 false, NULL, NULL, true },
    { 359,  70,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Rec. ptn.",  NULL,    NULL,       recordPattern,              false, NULL, NULL, true },
    { 359,  87,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Disk op.",   NULL,    NULL,       toggleDiskOpScreen,         false, NULL, NULL, true },
    { 359, 104,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Instr. Ed.", NULL,    NULL,       toggleInstEditor,           false, NULL, NULL, true },
    { 359, 121,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Smp. Ed.",   NULL,    NULL,       toggleSampleEditor,         false, NULL, NULL, true },
    { 359, 138,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Config",     NULL,    NULL,       showConfigScreen,           false, NULL, NULL, true },
    { 359, 155,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Help",       NULL,    NULL,       showHelpScreen,             false, NULL, NULL, true },
    /* extended pattern editor */
    { 115,  35,  46,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",       NULL,    NULL,       exitPatternEditorExtended,  false, NULL, NULL, true },

    // ------ INSTRUMENT SWITCHER PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,      line #2, funcOnDown,           funcOnUp
    { 590,   2,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "01-08",           NULL,    NULL,                 pbSetInstrBank1,  false, NULL, NULL, true },
    { 590,  19,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "09-10",           NULL,    NULL,                 pbSetInstrBank2,  false, NULL, NULL, true },
    { 590,  36,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "11-18",           NULL,    NULL,                 pbSetInstrBank3,  false, NULL, NULL, true },
    { 590,  53,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "19-20",           NULL,    NULL,                 pbSetInstrBank4,  false, NULL, NULL, true },
    { 590,  73,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "21-28",           NULL,    NULL,                 pbSetInstrBank5,  false, NULL, NULL, true },
    { 590,  90,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "29-30",           NULL,    NULL,                 pbSetInstrBank6,  false, NULL, NULL, true },
    { 590, 107,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "31-38",           NULL,    NULL,                 pbSetInstrBank7,  false, NULL, NULL, true },
    { 590, 124,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "39-40",           NULL,    NULL,                 pbSetInstrBank8,  false, NULL, NULL, true },
    { 590,   2,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "41-48",           NULL,    NULL,                 pbSetInstrBank9,  false, NULL, NULL, true },
    { 590,  19,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "49-50",           NULL,    NULL,                 pbSetInstrBank10, false, NULL, NULL, true },
    { 590,  36,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "51-58",           NULL,    NULL,                 pbSetInstrBank11, false, NULL, NULL, true },
    { 590,  53,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "59-60",           NULL,    NULL,                 pbSetInstrBank12, false, NULL, NULL, true },
    { 590,  73,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "61-68",           NULL,    NULL,                 pbSetInstrBank13, false, NULL, NULL, true },
    { 590,  90,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "69-70",           NULL,    NULL,                 pbSetInstrBank14, false, NULL, NULL, true },
    { 590, 107,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "71-78",           NULL,    NULL,                 pbSetInstrBank15, false, NULL, NULL, true },
    { 590, 124,  39,  16, false,   PUSHBUTTON_UNPRESSED, 3, "79-80",           NULL,    NULL,                 pbSetInstrBank16, false, NULL, NULL, true },
    { 590, 144,  39,  27, false,   PUSHBUTTON_UNPRESSED, 3, "Swap",            "Bank",  NULL,                 pbSwapInstrBank,  false, NULL, NULL, true },
    { 566,  99,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    sampleListScrollUp,   NULL,             false, NULL, NULL, true },
    { 566, 140,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    sampleListScrollDown, NULL,             false, NULL, NULL, true },

    // ------ NIBBLES SCREEN PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    { 568, 104,  61,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Play",       NULL,    NULL,       nibblesPlay,      false, NULL, NULL, true },
    { 568, 121,  61,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Help",       NULL,    NULL,       nibblesHelp,      false, NULL, NULL, true },
    { 568, 138,  61,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Highs",      NULL,    NULL,       nibblesHighScore, false, NULL, NULL, true },
    { 568, 155,  61,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",       NULL,    NULL,       nibblesExit,      false, NULL, NULL, true },

    // ------ ADVANCED EDIT PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    {   3, 138,  51,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Track",      NULL,    NULL,       remapTrack,   false, NULL, NULL, true },
    {  55, 138,  52,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Pattern",    NULL,    NULL,       remapPattern, false, NULL, NULL, true },
    {   3, 155,  51,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Song",       NULL,    NULL,       remapSong,    false, NULL, NULL, true },
    {  55, 155,  52,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Block",      NULL,    NULL,       remapBlock,   false, NULL, NULL, true },
      
    // ------ ABOUT SCREEN PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    {   5, 153,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",        NULL,    NULL,       exitAboutScreen, false, NULL, NULL, true },

    // ------ HELP SCREEN PUSHBUTTONS ------
    //x,   y,   w,   h,    visible, state,                  text line #1,      line #2, funcOnDown,     funcOnUp
    {   3, 155,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",            NULL,    NULL,           exitHelpScreen, false, NULL, NULL, true },
    { 611,   2,  18,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_UP_STRING,   NULL,    helpScrollUp,   NULL,           false, NULL, NULL, true },
    { 611, 158,  18,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_DOWN_STRING, NULL,    helpScrollDown, NULL,           false, NULL, NULL, true },

    // ------ PATTERN EDITOR PUSHBUTTONS ------
    //x,   y,   w,   h,    visible, state,                  text line #1,       line #2, funcOnDown,         funcOnUp
    {   3, 385,  25,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    scrollChannelLeft,  NULL,    false, NULL, NULL, true },
    { 604, 385,  25,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    scrollChannelRight, NULL,    false, NULL, NULL, true },

    // ------ TRANSPOSE PUSHBUTTONS ------
    //x,   y,   w,   h,    visible, state,                  text line #1,       line #2, funcOnDown, funcOnUp
    {  56, 110,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       trackTranspCurInsUp,   false, NULL, NULL, true },
    {  76, 110,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       trackTranspCurInsDn,   false, NULL, NULL, true },
    {  98, 110,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       trackTranspCurIns12Up, false, NULL, NULL, true },
    { 133, 110,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       trackTranspCurIns12Dn, false, NULL, NULL, true },
    { 175, 110,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       trackTranspAllInsUp,   false, NULL, NULL, true },
    { 195, 110,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       trackTranspAllInsDn,   false, NULL, NULL, true },
    { 217, 110,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       trackTranspAllIns12Up, false, NULL, NULL, true },
    { 252, 110,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       trackTranspAllIns12Dn, false, NULL, NULL, true },
    {  56, 125,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       pattTranspCurInsUp,    false, NULL, NULL, true },
    {  76, 125,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       pattTranspCurInsDn,    false, NULL, NULL, true },
    {  98, 125,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       pattTranspCurIns12Up,  false, NULL, NULL, true },
    { 133, 125,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       pattTranspCurIns12Dn,  false, NULL, NULL, true },
    { 175, 125,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       pattTranspAllInsUp,    false, NULL, NULL, true },
    { 195, 125,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       pattTranspAllInsDn,    false, NULL, NULL, true },
    { 217, 125,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       pattTranspAllIns12Up,  false, NULL, NULL, true },
    { 252, 125,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       pattTranspAllIns12Dn,  false, NULL, NULL, true },
    {  56, 140,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       songTranspCurInsUp,    false, NULL, NULL, true },
    {  76, 140,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       songTranspCurInsDn,    false, NULL, NULL, true },
    {  98, 140,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       songTranspCurIns12Up,  false, NULL, NULL, true },
    { 133, 140,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       songTranspCurIns12Dn,  false, NULL, NULL, true },
    { 175, 140,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       songTranspAllInsUp,    false, NULL, NULL, true },
    { 195, 140,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       songTranspAllInsDn,    false, NULL, NULL, true },
    { 217, 140,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       songTranspAllIns12Up,  false, NULL, NULL, true },
    { 252, 140,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       songTranspAllIns12Dn,  false, NULL, NULL, true },
    {  56, 155,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       blockTranspCurInsUp,   false, NULL, NULL, true },
    {  76, 155,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       blockTranspCurInsDn,   false, NULL, NULL, true },
    {  98, 155,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       blockTranspCurIns12Up, false, NULL, NULL, true },
    { 133, 155,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       blockTranspCurIns12Dn, false, NULL, NULL, true },
    { 175, 155,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "up",               NULL,    NULL,       blockTranspAllInsUp,   false, NULL, NULL, true },
    { 195, 155,  21,  16, false,   PUSHBUTTON_UNPRESSED, 3, "dn",               NULL,    NULL,       blockTranspAllInsDn,   false, NULL, NULL, true },
    { 217, 155,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12up",             NULL,    NULL,       blockTranspAllIns12Up, false, NULL, NULL, true },
    { 252, 155,  36,  16, false,   PUSHBUTTON_UNPRESSED, 3, "12dn",             NULL,    NULL,       blockTranspAllIns12Dn, false, NULL, NULL, true },

    // ------ SAMPLE EDITOR PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,            funcOnUp
    {   3, 331,  23,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_LEFT_STRING,  NULL,    scrollSampleDataLeft,  NULL,                false, NULL, NULL, true },
    { 606, 331,  23,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_RIGHT_STRING, NULL,    scrollSampleDataRight, NULL,                false, NULL, NULL, true },
    {  38, 356,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    sampPlayNoteUp,        NULL,                false, NULL, NULL, true },
    {  38, 368,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    sampPlayNoteDown,      NULL,                false, NULL, NULL, true },
    {   3, 382,  53,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Stop",             NULL,    NULL,                  smpEdStop,           false, NULL, NULL, true },
    {  57, 348,  55,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Wave",             NULL,    NULL,                  sampPlayWave,        false, NULL, NULL, true },
    {  57, 365,  55,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Range",            NULL,    NULL,                  sampPlayRange,       false, NULL, NULL, true },
    {  57, 382,  55,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Display",          NULL,    NULL,                  sampPlayDisplay,     false, NULL, NULL, true },
    { 118, 348,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Show r.",          NULL,    NULL,                  showRange,           false, NULL, NULL, true },
    { 118, 365,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Range all",        NULL,    NULL,                  rangeAll,            false, NULL, NULL, true },
    { 118, 382,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Sample",           NULL,    NULL,                  askToSample,         false, NULL, NULL, true },
    { 182, 348,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Zoom out",         NULL,    NULL,                  zoomSampleDataOut2x, false, NULL, NULL, true },
    { 182, 365,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Show all",         NULL,    NULL,                  showAll,             false, NULL, NULL, true },
    { 182, 382,  63,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Save rng.",        NULL,    NULL,                  saveRange,           false, NULL, NULL, true },
    { 251, 348,  43,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Cut",              NULL,    NULL,                  sampCut,             false, NULL, NULL, true },
    { 251, 365,  43,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Copy",             NULL,    NULL,                  sampCopy,            false, NULL, NULL, true },
    { 251, 382,  43,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Paste",            NULL,    NULL,                  sampPaste,           false, NULL, NULL, true },
    { 300, 348,  50,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Crop",             NULL,    NULL,                  sampCrop,            false, NULL, NULL, true },
    { 300, 365,  50,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Volume",           NULL,    NULL,                  sampleVolume,        false, NULL, NULL, true },
    { 300, 382,  50,  16, false,   PUSHBUTTON_UNPRESSED, 3, "X-Fade",           NULL,    NULL,                  sampXFade,           false, NULL, NULL, true },
    { 430, 348,  54,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",             NULL,    NULL,                  exitSampleEditor,    false, NULL, NULL, true },
    { 594, 348,  35,  13, false,   PUSHBUTTON_UNPRESSED, 3, "Clr S.",           NULL,    NULL,                  sampClear,           false, NULL, NULL, true },
    { 594, 360,  35,  13, false,   PUSHBUTTON_UNPRESSED, 3, "Min.",             NULL,    NULL,                  sampMin,             false, NULL, NULL, true },
    { 594, 373,  18,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_UP_STRING,    NULL,    sampRepeatUp,          NULL,                false, NULL, NULL, true },
    { 611, 373,  18,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_DOWN_STRING,  NULL,    sampRepeatDown,        NULL,                false, NULL, NULL, true },
    { 594, 385,  18,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_UP_STRING,    NULL,    sampReplenUp,          NULL,                false, NULL, NULL, true },
    { 611, 385,  18,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_DOWN_STRING,  NULL,    sampReplenDown,        NULL,                false, NULL, NULL, true },

    /* SAMPLE EDITOR EXTENSION */

    {   3, 138,  52,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Clr. c.bf", NULL,    NULL, clearCopyBuffer, false, NULL, NULL, true },
    {  56, 138,  49,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Conv",      NULL,    NULL, sampleConv,      false, NULL, NULL, true },
    { 106, 138,  49,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Echo",      NULL,    NULL, sampleEcho,      false, NULL, NULL, true },
    {   3, 155,  52,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Backw.",    NULL,    NULL, sampleBackwards, false, NULL, NULL, true },
    {  56, 155,  49,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Conv W",    NULL,    NULL, sampleConvW,     false, NULL, NULL, true },
    { 106, 155,  49,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Fix DC",    NULL,    NULL, fixDC,           false, NULL, NULL, true },
    { 161, 121,  60,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Copy ins.", NULL,    NULL, copyInstr,       false, NULL, NULL, true },
    { 222, 121,  66,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Copy smp.", NULL,    NULL, copySmp,         false, NULL, NULL, true },
    { 161, 138,  60,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Xchg ins.", NULL,    NULL, xchgInstr,       false, NULL, NULL, true },
    { 222, 138,  66,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Xchg smp.", NULL,    NULL, xchgSmp,         false, NULL, NULL, true },
    { 161, 155,  60,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Resample",  NULL,    NULL, sampleResample,  false, NULL, NULL, true },
    { 222, 155,  66,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Mix smp.",  NULL,    NULL, sampleMixSample, false, NULL, NULL, true },

    // ------ INSTRUMENT EDITOR PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,     funcOnUp
    { 200, 175,  23,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_1_STRING,     NULL,    NULL,           volPreDef1,     false, NULL, NULL, true },
    { 222, 175,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_2_STRING,     NULL,    NULL,           volPreDef2,     false, NULL, NULL, true },
    { 245, 175,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_3_STRING,     NULL,    NULL,           volPreDef3,     false, NULL, NULL, true },
    { 268, 175,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_4_STRING,     NULL,    NULL,           volPreDef4,     false, NULL, NULL, true },
    { 291, 175,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_5_STRING,     NULL,    NULL,           volPreDef5,     false, NULL, NULL, true },
    { 314, 175,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_6_STRING,     NULL,    NULL,           volPreDef6,     false, NULL, NULL, true },
    { 200, 262,  23,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_1_STRING,     NULL,    NULL,           panPreDef1,     false, NULL, NULL, true },
    { 222, 262,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_2_STRING,     NULL,    NULL,           panPreDef2,     false, NULL, NULL, true },
    { 245, 262,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_3_STRING,     NULL,    NULL,           panPreDef3,     false, NULL, NULL, true },
    { 268, 262,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_4_STRING,     NULL,    NULL,           panPreDef4,     false, NULL, NULL, true },
    { 291, 262,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_5_STRING,     NULL,    NULL,           panPreDef5,     false, NULL, NULL, true },
    { 314, 262,  24,  12, false,   PUSHBUTTON_UNPRESSED, 3, SMALL_6_STRING,     NULL,    NULL,           panPreDef6,     false, NULL, NULL, true },
    { 341, 175,  47,  16, false,   PUSHBUTTON_UNPRESSED, 4, "Add",              NULL,    volEnvAdd,      NULL,           false, NULL, NULL, true },
    { 389, 175,  46,  16, false,   PUSHBUTTON_UNPRESSED, 4, "Del",              NULL,    volEnvDel,      NULL,           false, NULL, NULL, true },
    { 398, 204,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    volEnvSusUp,    NULL,           false, NULL, NULL, true },
    { 416, 204,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    volEnvSusDown,  NULL,           false, NULL, NULL, true },
    { 398, 231,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    volEnvRepSUp,   NULL,           false, NULL, NULL, true },
    { 416, 231,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    volEnvRepSDown, NULL,           false, NULL, NULL, true },
    { 398, 245,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    volEnvRepEUp,   NULL,           false, NULL, NULL, true },
    { 416, 245,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    volEnvRepEDown, NULL,           false, NULL, NULL, true },
    { 341, 262,  47,  16, false,   PUSHBUTTON_UNPRESSED, 4, "Add",              NULL,    panEnvAdd,      NULL,           false, NULL, NULL, true },
    { 389, 262,  46,  16, false,   PUSHBUTTON_UNPRESSED, 4, "Del",              NULL,    panEnvDel,      NULL,           false, NULL, NULL, true },
    { 398, 291,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    panEnvSusUp,    NULL,           false, NULL, NULL, true },
    { 416, 291,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    panEnvSusDown,  NULL,           false, NULL, NULL, true },
    { 398, 318,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    panEnvRepSUp,   NULL,           false, NULL, NULL, true },
    { 416, 318,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    panEnvRepSDown, NULL,           false, NULL, NULL, true },
    { 398, 332,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,    NULL,    panEnvRepEUp,   NULL,           false, NULL, NULL, true },
    { 416, 332,  19,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,  NULL,    panEnvRepEDown, NULL,           false, NULL, NULL, true },
    { 521, 175,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    volDown,        NULL,           false, NULL, NULL, true },
    { 606, 175,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    volUp,          NULL,           false, NULL, NULL, true },
    { 521, 189,  23,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_LEFT_STRING,  NULL,    panDown,        NULL,           false, NULL, NULL, true },
    { 606, 189,  23,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_RIGHT_STRING, NULL,    panUp,          NULL,           false, NULL, NULL, true },
    { 521, 203,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    ftuneDown,      NULL,           false, NULL, NULL, true },
    { 606, 203,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    ftuneUp,        NULL,           false, NULL, NULL, true },
    { 521, 220,  23,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_LEFT_STRING,  NULL,    fadeoutDown,    NULL,           false, NULL, NULL, true },
    { 606, 220,  23,  13, false,   PUSHBUTTON_UNPRESSED, 0, ARROW_RIGHT_STRING, NULL,    fadeoutUp,      NULL,           false, NULL, NULL, true },
    { 521, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    vibSpeedDown,   NULL,           false, NULL, NULL, true },
    { 606, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    vibSpeedUp,     NULL,           false, NULL, NULL, true },
    { 521, 248,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    vibDepthDown,   NULL,           false, NULL, NULL, true },
    { 606, 248,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    vibDepthUp,     NULL,           false, NULL, NULL, true },
    { 521, 262,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    vibSweepDown,   NULL,           false, NULL, NULL, true },
    { 606, 262,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    vibSweepUp,     NULL,           false, NULL, NULL, true },
    { 570, 276,  59,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",             NULL,    NULL,           exitInstEditor, false, NULL, NULL, true },
    { 441, 312,  94,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Octave up",        NULL,    relToneOctUp,   NULL,           false, NULL, NULL, true },
    { 536, 312,  93,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Halftone up",      NULL,    relToneUp,      NULL,           false, NULL, NULL, true },
    { 441, 329,  94,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Octave down",      NULL,    relToneOctDown, NULL,           false, NULL, NULL, true },
    { 536, 329,  93,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Halftone down",    NULL,    relToneDown,    NULL,           false, NULL, NULL, true },

    // ------ INSTRUMENT EDITOR EXTENSION PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,   funcOnUp
    { 172, 130,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    midiChDown,   NULL,    false, NULL, NULL, true },
    { 265, 130,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    midiChUp,     NULL,    false, NULL, NULL, true },
    { 172, 144,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    midiPrgDown,  NULL,    false, NULL, NULL, true },
    { 265, 144,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    midiPrgUp,    NULL,    false, NULL, NULL, true },
    { 172, 158,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    midiBendDown, NULL,    false, NULL, NULL, true },
    { 265, 158,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    midiBendUp,   NULL,    false, NULL, NULL, true },

    // ------ TRIM SCREEN PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    { 139, 155,  74,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Calculate",  NULL,    NULL,       pbTrimCalc,    false, NULL, NULL, true },
    { 214, 155,  74,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Trim",       NULL,    NULL,       pbTrimTrim,    false, NULL, NULL, true },

    // ------ CONFIG LEFT PANEL PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,   line #2, funcOnDown, funcOnUp
    {   3, 104, 104,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Reset config.", NULL,    NULL,      resetConfig2,     false, NULL, NULL, true },
    {   3, 121, 104,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Load config.",  NULL,    NULL,      loadConfig2,      false, NULL, NULL, true },
    {   3, 138, 104,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Save config.",  NULL,    NULL,      saveConfig2,      false, NULL, NULL, true },
    {   3, 155, 104,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",          NULL,    NULL,      exitConfigScreen, false, NULL, NULL, true },

    // ------ CONFIG AUDIO PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,      funcOnUp
    { 326,   2,  57,  13, false,   PUSHBUTTON_UNPRESSED, 3, "Re-scan",          NULL,    NULL,                       rescanAudioDevices, false, NULL, NULL, true },
    { 365,  16,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,    NULL,    scrollAudOutputDevListUp,   NULL,               false, NULL, NULL, true },
    { 365,  72,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING,  NULL,    scrollAudOutputDevListDown, NULL,               false, NULL, NULL, true },
    { 365, 103,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,    NULL,    scrollAudInputDevListUp,    NULL,               false, NULL, NULL, true },
    { 365, 158,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING,  NULL,    scrollAudInputDevListDown,  NULL,               false, NULL, NULL, true },
    { 508, 132,  21,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    configAmpDown,              NULL,               false, NULL, NULL, true },
    { 608, 132,  21,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    configAmpUp,                NULL,               false, NULL, NULL, true },
    { 508, 158,  21,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_LEFT_STRING,  NULL,    configMasterVolDown,        NULL,               false, NULL, NULL, true },
    { 608, 158,  21,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_RIGHT_STRING, NULL,    configMasterVolUp,          NULL,               false, NULL, NULL, true },

    // ------ CONFIG LAYOUT PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,         funcOnUp
    { 513,  15,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    configPalRDown,     NULL,     false, NULL, NULL, true },
    { 606,  15,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    configPalRUp,       NULL,     false, NULL, NULL, true },
    { 513,  29,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    configPalGDown,     NULL,     false, NULL, NULL, true },
    { 606,  29,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    configPalGUp,       NULL,     false, NULL, NULL, true },
    { 513,  43,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    configPalBDown,     NULL,     false, NULL, NULL, true },
    { 606,  43,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    configPalBUp,       NULL,     false, NULL, NULL, true },
    { 513,  71,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,  NULL,    configPalContDown,  NULL,     false, NULL, NULL, true },
    { 606,  71,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING, NULL,    configPalContUp,    NULL,     false, NULL, NULL, true },

    // ------ CONFIG MISCELLANEOUS PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,                   line #2, funcOnDown,         funcOnUp
    { 113, 155,  93,  16, false,   PUSHBUTTON_UNPRESSED, 3, editor.ui.fullscreenButtonText, NULL,    NULL,                toggleFullScreen, false, NULL, NULL, true },
    { 370, 121,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,                NULL,    configQuantizeUp,    NULL, false, NULL, NULL, true },
    { 387, 121,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,              NULL,    configQuantizeDown,  NULL, false, NULL, NULL, true },
    { 594, 107,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_UP_STRING,                NULL,    configMIDIChnUp,     NULL, false, NULL, NULL, true },
    { 611, 107,  18,  13, false,   PUSHBUTTON_UNPRESSED, 4, ARROW_DOWN_STRING,              NULL,    configMIDIChnDown,   NULL, false, NULL, NULL, true },
    { 594, 121,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,                NULL,    configMIDITransUp,   NULL, false, NULL, NULL, true },
    { 611, 121,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING,              NULL,    configMIDITransDown, NULL, false, NULL, NULL, true },
    { 556, 158,  22,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,              NULL,    configMIDISensDown,  NULL, false, NULL, NULL, true },
    { 607, 158,  22,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,             NULL,    configMIDISensUp,    NULL, false, NULL, NULL, true },

    // ------ CONFIG MIDI PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,       line #2, funcOnDown,                 funcOnUp
    { 483,   2,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,    NULL,    scrollMidiInputDevListUp,   NULL, false, NULL, NULL, true },
    { 483, 158,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING,  NULL,    scrollMidiInputDevListDown, NULL, false, NULL, NULL, true },

    // ------ DISK OP. PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    {  70,   2,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Save",       NULL,    NULL,       pbDiskOpSave,    false, NULL, NULL, true },
    {  70,  19,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Delete",     NULL,    NULL,       pbDiskOpDelete,  false, NULL, NULL, true },
    {  70,  36,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Rename",     NULL,    NULL,       pbDiskOpRename,  false, NULL, NULL, true },
    {  70,  53,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Makedir",    NULL,    NULL,       pbDiskOpMakeDir, false, NULL, NULL, true },
    {  70,  70,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Refresh",    NULL,    NULL,       pbDiskOpRefresh, false, NULL, NULL, true },
    {  70,  87,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Set path",   NULL,    NULL,       pbDiskOpSetPath, false, NULL, NULL, true },
    {  70, 104,  58,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Show all",   NULL,    NULL,       pbDiskOpShowAll, false, NULL, NULL, true },
    {  70, 121,  58,  19, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",       NULL,    NULL,       pbDiskOpExit,    false, NULL, NULL, true },

#ifdef _WIN32
    { 134,   2,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, ".\001",      NULL,    NULL,       pbDiskOpParent,  false, NULL, NULL, true },
    { 134,  16,  31,  12, false,   PUSHBUTTON_UNPRESSED, 3, "\\",         NULL,    NULL,       pbDiskOpRoot,    false, NULL, NULL, true },
#else
    { 134,   2,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "../",        NULL,    NULL,       pbDiskOpParent,  false, NULL, NULL, true },
    { 134,  16,  31,  12, false,   PUSHBUTTON_UNPRESSED, 3, "/",          NULL,    NULL,       pbDiskOpRoot,    false, NULL, NULL, true },
#endif

#ifdef _WIN32
    //x,   y,   w,   h,   visible, state,                   text line #1, line #2, funcOnDown, funcOnUp
    { 134,  29,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive1, false, NULL, NULL, true },
    { 134,  43,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive2, false, NULL, NULL, true },
    { 134,  57,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive3, false, NULL, NULL, true },
    { 134,  71,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive4, false, NULL, NULL, true },
    { 134,  85,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive5, false, NULL, NULL, true },
    { 134,  99,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive6, false, NULL, NULL, true },
    { 134, 113,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive7, false, NULL, NULL, true },
    { 134, 127,  31,  13, false,   PUSHBUTTON_UNPRESSED, 3, "",           NULL,    NULL,       pbDiskOpDrive8, false, NULL, NULL, true },
#endif

    { 335,   2,  18,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_UP_STRING,   NULL, pbDiskOpListUp,   NULL, false, NULL, NULL, true },
    { 335, 158,  18,  13, false,   PUSHBUTTON_UNPRESSED, 1, ARROW_DOWN_STRING, NULL, pbDiskOpListDown, NULL, false, NULL, NULL, true },

    // ------ WAV RENDERER PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,      line #2, funcOnDown,         funcOnUp
    {   3, 111,  73,  43, false,   PUSHBUTTON_UNPRESSED, 3, "RECORD",          NULL,    NULL,               pbWavRender, false, NULL, NULL, true },
    {   3, 155,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",            NULL,    NULL,               pbWavExit,   false, NULL, NULL, true },
    { 253, 114,  18,  13, false,   PUSHBUTTON_UNPRESSED, 6, ARROW_UP_STRING,   NULL,    pbWavFreqUp,        NULL,        false, NULL, NULL, true },
    { 270, 114,  18,  13, false,   PUSHBUTTON_UNPRESSED, 6, ARROW_DOWN_STRING, NULL,    pbWavFreqDown,      NULL,        false, NULL, NULL, true },
    { 253, 128,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbWavAmpUp,         NULL,        false, NULL, NULL, true },
    { 270, 128,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbWavAmpDown,       NULL,        false, NULL, NULL, true },
    { 253, 142,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbWavSongStartUp,   NULL,        false, NULL, NULL, true },
    { 270, 142,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbWavSongStartDown, NULL,        false, NULL, NULL, true },
    { 253, 156,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_UP_STRING,   NULL,    pbWavSongEndUp,     NULL,        false, NULL, NULL, true },
    { 270, 156,  18,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_DOWN_STRING, NULL,    pbWavSongEndDown,   NULL,        false, NULL, NULL, true },

    // ------ SYSTEM REQUEST PUSHBUTTONS ------
    //x,   y,   w,   h,   visible, state,                   text line #1,  line #2, funcOnDown, funcOnUp
    { 126, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "All",         NULL,    NULL,       pbZapAll,                 false, NULL, NULL, true },
    { 226, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Song",        NULL,    NULL,       pbZapSong,                false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Instruments", NULL,    NULL,       pbZapInstr,               false, NULL, NULL, true },
    { 426, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Cancel",      NULL,    NULL,       hideSystemRequest,        false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       doTranspose,              false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       resetConfig,              false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       quitProgram,              false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "No",          NULL,    NULL,       hideSystemRequest,        false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       quitProgram,              false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Cancel",      NULL,    NULL,       hideSystemRequest,        false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "=(",          NULL,    NULL,       quitProgram,              false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Rules",       NULL,    NULL,       hideSystemRequest,        false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       hideSystemRequest,        false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       shrinkPattern,            false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       setNewLenAndPastePattern, false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "No",          NULL,    NULL,       pastePatternNoLenCheck,   false, NULL, NULL, true },

    /* nibbles */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       nibblesRestartYes,        false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       nibblesExit2,             false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       nibblesGameOverOK,        false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       nibblesLevelFinishedOK,   false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       nibblesPlayer1NameOK,     false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       nibblesPlayer2NameOK,     false, NULL, NULL, true },
    { 276, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       nibblesPlayerDiedOK,      false, NULL, NULL, true },

    /* sampler */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       clearCurSample,          false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       convSampleTo8Bit,        false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "No",          NULL,    NULL,       convSampleTo8BitCancel,  false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL,       convSampleTo16Bit,       false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "No",          NULL,    NULL,       convSampleTo16BitCancel, false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL,       minimizeSample,          false, NULL, NULL, true },

    { 125, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Read left",   NULL,    NULL,       stereoSampleReadLeft,  false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Read right",  NULL,    NULL,       stereoSampleReadRight, false, NULL, NULL, true },
    { 325, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Convert",     NULL,    NULL,       stereoSampleConvert,   false, NULL, NULL, true },
    { 425, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Abort",       NULL,    NULL,       hideSystemRequest,     false, NULL, NULL, true },

    /* sample volume box */
    { 171, 262,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Apply",             NULL, NULL, sampleChangeVolume, false, NULL, NULL, true },
    { 245, 262, 143,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Get maximum scale", NULL, NULL, sampleGetMaxVolume, false, NULL, NULL, true },
    { 389, 262,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",              NULL, NULL, hideSystemRequest,  false, NULL, NULL, true },
    { 292, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbSampStartVolDown, NULL, false, NULL, NULL, true },
    { 439, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbSampStartVolUp,   NULL, false, NULL, NULL, true },
    { 292, 248,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbSampEndVolDown,   NULL, false, NULL, NULL, true },
    { 439, 248,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbSampEndVolUp,     NULL, false, NULL, NULL, true },

    /* sample resample box */
    { 214, 264,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Apply",             NULL, NULL, resampleSample,      false, NULL, NULL, true },
    { 345, 264,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",              NULL, NULL, hideSystemRequest,   false, NULL, NULL, true },
    { 314, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbResampleTonesDown, NULL, false, NULL, NULL, true },
    { 395, 234,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbResampleTonesUp,   NULL, false, NULL, NULL, true },

    /* mix sample box */
    { 197, 258,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Apply",             NULL, NULL, mixSample,         false, NULL, NULL, true },
    { 361, 258,  73,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Exit",              NULL, NULL, hideSystemRequest, false, NULL, NULL, true },
    { 322, 244,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbMixBalanceDown,  NULL, false, NULL, NULL, true },
    { 411, 244,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbMixBalanceUp,    NULL, false, NULL, NULL, true },

    /* echo box */
    { 345, 266,  56,  16, false,   PUSHBUTTON_UNPRESSED, 1, "Apply",             NULL, NULL, createEcho,        false, NULL, NULL, true },
    { 402, 266,  55,  16, false,   PUSHBUTTON_UNPRESSED, 1, "Exit",              NULL, NULL, hideSystemRequest, false, NULL, NULL, true },
    { 345, 224,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbEchoNumDown,     NULL, false, NULL, NULL, true },
    { 434, 224,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbEchoNumUp,       NULL, false, NULL, NULL, true },
    { 345, 238,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbEchoDistDown,    NULL, false, NULL, NULL, true },
    { 434, 238,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbEchoDistUp,      NULL, false, NULL, NULL, true },
    { 345, 252,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_LEFT_STRING,   NULL, pbEchoFadeoutDown, NULL, false, NULL, NULL, true },
    { 434, 252,  23,  13, false,   PUSHBUTTON_UNPRESSED, 3, ARROW_RIGHT_STRING,  NULL, pbEchoFadeoutUp,   NULL, false, NULL, NULL, true },

    /* save range box */
    { 226, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, saveRange2,   false, NULL, NULL, true },

    /* disk op */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL, diskOpDelete,          false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, diskOpRenameAnsi,      false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, diskOpMakeDirAnsi,     false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, diskOpSetPathAnsi,     false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL, diskOpSave2,           false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL, wavRenderOverwrite,    false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL, openFile2,             false, NULL, NULL, true },
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",         NULL,    NULL, loadDroppedFile2,      false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "No",          NULL,    NULL, cancelLoadDroppedFile, false, NULL, NULL, true },

    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, clearCurInstr, false, NULL, NULL, true },

    /* scale fade volume */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "OK",          NULL,    NULL, handleScaleFadeVolume,  false, NULL, NULL, true },
    { 326, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Cancel",      NULL,    NULL, cancelScaleFadeVolume,  false, NULL, NULL, true },

    /* trim */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",        NULL,    NULL, trim,  false, NULL, NULL, true },

    /* sampling */
    { 225, 291,  81,  16, false,   PUSHBUTTON_UNPRESSED, 3, "Yes",        NULL,    NULL, srStartSampling,  false, NULL, NULL, true }
};

void drawPushButton(uint16_t pushButtonID)
{
    uint8_t state;
    uint16_t x, y, w, h, textX, textY, textW;
    pushButton_t *b;

    MY_ASSERT(pushButtonID < NUM_PUSHBUTTONS)

    b = &pushButtons[pushButtonID];
    if (!b->visible)
        return;

    state = b->state;

    x = b->x;
    y = b->y;
    w = b->w;
    h = b->h;

    MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H) && (w >= 4) && (h >= 4))

    if (b->bitmapFlag)
    {
        blitFast(x, y, (state == PUSHBUTTON_UNPRESSED) ? b->bitmapUnpressed : b->bitmapPressed, w, h);
        return;
    }

    /* fill button background */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* draw outer border */
    hLine(x,         y,         w, PAL_BCKGRND);
    hLine(x,         y + h - 1, w, PAL_BCKGRND);
    vLine(x,         y,         h, PAL_BCKGRND);
    vLine(x + w - 1, y,         h, PAL_BCKGRND);

    /* draw inner borders */
    if (state == PUSHBUTTON_UNPRESSED)
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

    /* render button text(s) */
    if ((b->caption != NULL) && (*b->caption != '\0'))
    {
        /* button text #2 */
        if ((b->caption2 != NULL) && (*b->caption2 != '\0'))
        {
            textW = getTextWidth(b->caption2, FONT_TYPE1);
            textX = x + ((w - textW) / 2);
            textY = y + 6 + ((h - (FONT1_CHAR_H - 2)) / 2);

            if (state == PUSHBUTTON_PRESSED)
                textOut(textX + 1, textY + 1, PAL_BTNTEXT, b->caption2);
            else
                textOut(textX, textY, PAL_BTNTEXT, b->caption2);

            y -= 5; /* if two text lines, bias y position of first (upper) text */
        }

        /* button text #1 */
        textW = getTextWidth(b->caption, FONT_TYPE1);
        textX = x + ((w - textW) / 2);
        textY = y + ((h - (FONT1_CHAR_H - 2)) / 2);

        if (state == PUSHBUTTON_PRESSED)
            textOut(textX + 1, textY + 1, PAL_BTNTEXT, b->caption);
        else
            textOut(textX, textY, PAL_BTNTEXT, b->caption);
    }
}

void showPushButton(uint16_t pushButtonID)
{
    MY_ASSERT(pushButtonID < NUM_PUSHBUTTONS)

    pushButtons[pushButtonID].visible = true;
    drawPushButton(pushButtonID);
}

void hidePushButton(uint16_t pushButtonID)
{
    MY_ASSERT(pushButtonID < NUM_PUSHBUTTONS)

    pushButtons[pushButtonID].visible = false;
}

void handlePushButtonsWhileMouseDown(void)
{
    int8_t buttonDelay;
    pushButton_t *pushButton;

    MY_ASSERT((mouse.lastUsedObjectID >= 0) && (mouse.lastUsedObjectID < NUM_PUSHBUTTONS))

    pushButton = &pushButtons[mouse.lastUsedObjectID];
    if (!pushButton->visible)
        return;

    pushButton->state = PUSHBUTTON_UNPRESSED;
    if ((mouse.x >= pushButton->x) && (mouse.x < (pushButton->x + pushButton->w)))
    {
        if ((mouse.y >= pushButton->y) && (mouse.y < (pushButton->y + pushButton->h)))
            pushButton->state = PUSHBUTTON_PRESSED;
    }

    if ((mouse.lastX != mouse.x) || (mouse.lastY != mouse.y))
    {
        mouse.lastX = mouse.x;
        mouse.lastY = mouse.y;

        drawPushButton(mouse.lastUsedObjectID);
    }

    if (pushButton->callbackFuncOnDown != NULL)
    {
        /* long delay before repeat */
        if (pushButton->preDelayFlag && mouse.firstTimePressingButton)
        {
            if (++mouse.buttonCounter >= BUTTON_DOWN_DELAY)
            {
                mouse.buttonCounter = 0;
                mouse.firstTimePressingButton = false;
            }
            else
            {
                return; /* we're delaying */
            }
        }

        if (pushButton->state == PUSHBUTTON_PRESSED)
        {
            if (mouse.rightButtonPressed)
                buttonDelay = pushButton->delayFrames / 2;
            else
                buttonDelay = pushButton->delayFrames;

            /* main repeat delay */
            if (++mouse.buttonCounter >= buttonDelay)
            {
                mouse.buttonCounter = 0;
                if (pushButton->callbackFuncOnDown != NULL)
                    (pushButton->callbackFuncOnDown)();
            }
        }
    }
}

int8_t testPushButtonMouseDown(void)
{
    uint16_t i;
    pushButton_t *pushButton;
    sysReq_t *sysReq;

    if (editor.ui.systemRequestShown) /* only react to buttons inside the box when a system request is shown */
    {
        MY_ASSERT(editor.ui.systemRequestID < NUM_SYSREQS)

        sysReq = &sysReqs[editor.ui.systemRequestID];
        for (i = 0; i < sysReq->numButtons; ++i)
        {
            MY_ASSERT(sysReq->buttonIDs[i] < NUM_PUSHBUTTONS)

            pushButton = &pushButtons[sysReq->buttonIDs[i]];
            if ((mouse.x >= pushButton->x) && (mouse.x < (pushButton->x + pushButton->w)))
            {
                if ((mouse.y >= pushButton->y) && (mouse.y < (pushButton->y + pushButton->h)))
                {
                    mouse.lastUsedObjectID   = sysReq->buttonIDs[i];
                    mouse.lastUsedObjectType = OBJECT_PUSHBUTTON;

                    if (!mouse.rightButtonPressed)
                    {
                        mouse.firstTimePressingButton = true;
                        mouse.buttonCounter = 0;

                        pushButton->state = PUSHBUTTON_PRESSED;
                        drawPushButton(i);

                        if (pushButton->callbackFuncOnDown != NULL)
                           (pushButton->callbackFuncOnDown)();
                    }

                    return (true);
                }
            }
        }
    }
    else
    {
        for (i = 0; i < NUM_PUSHBUTTONS; ++i)
        {
            pushButton = &pushButtons[i];
            if (pushButton->visible)
            {
                if ((mouse.x >= pushButton->x) && (mouse.x < (pushButton->x + pushButton->w)))
                {
                    if ((mouse.y >= pushButton->y) && (mouse.y < (pushButton->y + pushButton->h)))
                    {
                        mouse.lastUsedObjectID   = i;
                        mouse.lastUsedObjectType = OBJECT_PUSHBUTTON;

                        if (!mouse.rightButtonPressed)
                        {
                            mouse.firstTimePressingButton = true;
                            mouse.buttonCounter = 0;

                            pushButton->state = PUSHBUTTON_PRESSED;
                            drawPushButton(i);

                            if (pushButton->callbackFuncOnDown != NULL)
                                (pushButton->callbackFuncOnDown)();
                        }

                        return (true);
                    }
                }
            }
        }
    }

    return (false);
}

void testPushButtonMouseRelease(void)
{
    pushButton_t *pushButton;

    if (mouse.lastUsedObjectType == OBJECT_PUSHBUTTON)
    {
        if (mouse.lastUsedObjectID != OBJECT_ID_NONE)
        {
            MY_ASSERT(mouse.lastUsedObjectID < NUM_PUSHBUTTONS)

            pushButton = &pushButtons[mouse.lastUsedObjectID];
            if (pushButton->visible)
            {
                if ((mouse.x >= pushButton->x) && (mouse.x < (pushButton->x + pushButton->w)))
                {
                    if ((mouse.y >= pushButton->y) && (mouse.y < (pushButton->y + pushButton->h)))
                    {
                        pushButton->state = PUSHBUTTON_UNPRESSED;
                        drawPushButton(mouse.lastUsedObjectID);

                        if (pushButton->callbackFuncOnUp != NULL)
                           (pushButton->callbackFuncOnUp)();
                    }
                }
            }
        }
    }
}
