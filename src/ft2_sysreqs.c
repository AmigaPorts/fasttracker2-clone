/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_mouse.h"
#include "ft2_config.h"
#include "ft2_sample_ed.h"
#include "ft2_nibbles.h"
#include "ft2_pattern_ed.h"
#include "ft2_sample_loader.h"
#include "ft2_diskop.h"
#include "ft2_wav_renderer.h"
#include "ft2_trim.h"
#include "ft2_sampling.h"
#include "ft2_module_loader.h"
#include "ft2_keyboard.h"
#include "ft2_edit.h"
#include "ft2_textboxes.h"

#define MAX_SYSREQ_QUEUE_LEN 10

/* System Requests are really hardcoded in this clone.
** TODO: Make less hardcoded so that we don't need so many specific checks?
*/

static uint8_t queueLen, queuePos;
static uint16_t queue[MAX_SYSREQ_QUEUE_LEN];

sysReq_t sysReqs[NUM_SYSREQS] =
{
    /* KILL SYSTEM REQUEST */
    { 488, "Total devastation of the...", {PB_SYSREQ_KILL_ALL, PB_SYSREQ_KILL_SONG, PB_SYSREQ_KILL_INSTR, PB_SYSREQ_KILL_CANCEL}, 4, SYSREQ_REQUEST},

    /* TRANSPOE REQUESTS */
    { 344, editor.ui.transpDelNotesText, {PB_SYSREQ_TRANSP_DEL_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},

    /* CONFIG */
    { 520, "WARNING: Are you sure you want to completely reset your FT2 configuration?", {PB_SYSREQ_CONFIG_RESET_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},

    /* EXIT SYSTEM REQUESTS */
    { 584, "You have unsaved changes in your song. Do you still want to quit and lose ALL changes?", {PB_SYSREQ_EXIT_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 296, "Do you really want to quit?", {PB_SYSREQ_EXIT_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},

    /* SHRINK/EXPAND PATTERN */
    { 296, "Shrink pattern?", {PB_SYSREQ_SHRINK_PATT_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 376, "Change pattern length to copybuffer's length?", {PB_SYSREQ_PASTEPATT_LEN_YES, PB_SYSREQ_PASTEPATT_LEN_NO}, 2, SYSREQ_REQUEST},

    /* CLEAR INSTRUMENT */
    { 296, "Clear instrument?", {PB_SYSREQ_INSTR_CLEAR_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_REQUEST},

    /* NIBBLES */
    { 344, "Restart the current game of nibbles?", { PB_SYSREQ_NIB_RESTART_YES, PB_SYSREQ_EXIT_NO }, 2, SYSREQ_NIBBLES },
    { 296, "Quit current game of nibbles?", { PB_SYSREQ_NIB_EXIT_YES, PB_SYSREQ_EXIT_NO }, 2, SYSREQ_REQUEST },
    { 296, "No help available during play.", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_REQUEST },
    { 344, "No highscoretable is available during play.", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_REQUEST },
    { 200, "Player 1 died!", { PB_SYSREQ_NIB_PLAYER_DIED_OK }, 1, SYSREQ_NIBBLES },
    { 200, "Player 2 died!", { PB_SYSREQ_NIB_PLAYER_DIED_OK }, 1, SYSREQ_NIBBLES },
    { 200, "Both players died!", { PB_SYSREQ_NIB_PLAYER_DIED_OK }, 1, SYSREQ_NIBBLES },
    { 200, "GAME OVER", { PB_SYSREQ_NIB_GAME_OVER_OK }, 1, SYSREQ_NIBBLES },
    { 200, editor.ui.nibblesLvlText, { PB_SYSREQ_NIB_LEVEL_FINISHED_OK }, 1, SYSREQ_NIBBLES },
    { 296, NULL, { PB_SYSREQ_NIB_PLAYER1_NAME_OK }, 1, SYSREQ_NIBBLES1 },
    { 296, NULL, { PB_SYSREQ_NIB_PLAYER2_NAME_OK }, 1, SYSREQ_NIBBLES2 },
    { 296, "Eternal lives activated!", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_NIBBLES3 },
    { 296, "Eternal lives deactivated!", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_NIBBLES3 },
    { 440, "\"Surround\" is not appropriate in one-player mode.", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_NIBBLES },

    /* SAMPLE EDITOR */
    { 296, "Clear sample?", {PB_SYSREQ_SAMP_CLEAR_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_REQUEST},
    { 296, "Convert sampledata?", {PB_SYSREQ_SAMP_8BIT_CONV_YES, PB_SYSREQ_SAMP_8BIT_CONV_NO}, 2, SYSREQ_REQUEST},
    { 296, "Convert sampledata?", {PB_SYSREQ_SAMP_16BIT_CONV_YES, PB_SYSREQ_SAMP_16BIT_CONV_NO}, 2, SYSREQ_REQUEST},
    { 296, "Minimize sample?", {PB_SYSREQ_SAMP_MIN_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_REQUEST},
    { 440, "This is a stereo sample...", {PB_SYSREQ_SAMP_READ_LEFT, PB_SYSREQ_SAMP_READ_RIGHT, PB_SYSREQ_SAMP_CONVERT, PB_SYSREQ_SAMP_ABORT}, 4, SYSREQ_REQUEST},
    { 0, "", {PB_SYSREQ_SAMPVOL_APPLY, PB_SYSREQ_SAMPVOL_GETMAX, PB_SYSREQ_SAMPVOL_EXIT,
                  PB_SYSREQ_SAMPVOL_START_DOWN, PB_SYSREQ_SAMPVOL_START_UP, PB_SYSREQ_SAMPVOL_END_DOWN, PB_SYSREQ_SAMPVOL_END_UP}, 7, 0},
    { 0, "", {PB_SYSREQ_RESAMPLE_APPLY, PB_SYSREQ_RESAMPLE_EXIT, PB_SYSREQ_RESAMPLE_UP, PB_SYSREQ_RESAMPLE_DOWN}, 4, 0},
    { 0, "", {PB_SYSREQ_MIX_SAMPLE_APPLY, PB_SYSREQ_MIX_SAMPLE_EXIT, PB_SYSREQ_MIX_BALANCE_UP, PB_SYSREQ_MIX_BALANCE_DOWN}, 4, 0},
    { 0, "", {PB_SYSREQ_ECHO_APPLY, PB_SYSREQ_ECHO_EXIT, PB_SYSREQ_ECHO_NECHO_DOWN, PB_SYSREQ_ECHO_NECHO_UP,
               PB_SYSREQ_ECHO_DIST_DOWN, PB_SYSREQ_ECHO_DIST_UP, PB_SYSREQ_ECHO_FADEOUT_DOWN, PB_SYSREQ_ECHO_FADEOUT_UP}, 8, 0},
    { 296, "", {PB_SYSREQ_SAVE_RANGE_YES, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_SAVERANGE},

    /* DISK OP */
    { 376, "", {PB_SYSREQ_DISKOP_DEL_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 344, "", {PB_SYSREQ_DISKOP_RENAME_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_RENAMEDIR},
    { 344, "", {PB_SYSREQ_DISKOP_RENAME_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_RENAMEFILE},
    { 344, "", {PB_SYSREQ_DISKOP_MAKEDIR_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_MAKEDIR},
    { 344, "", {PB_SYSREQ_DISKOP_SETPATH_OK, PB_SYSREQ_EXIT_CANCEL}, 2, SYSREQ_SETPATH},
    { 344, "", {PB_SYSREQ_DISKOP_OVERWRITE_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 344, "", {PB_SYSREQ_WAV_OVERWRITE_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 520, "You have unsaved changes in your song. Load new song and lose all changes?", {PB_SYSREQ_DISKOP_LOADMOD_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},
    { 520, "You have unsaved changes in your song. Load new song and lose all changes?", {PB_SYSREQ_DROP_LOADMOD_YES, PB_SYSREQ_DISKOP_LOADMOD_NO}, 2, SYSREQ_REQUEST},

    /* SCALE FADE VOLUME */
    { 375, "", {PB_SCALE_FADE_VOL_OK, PB_SCALE_FADE_VOL_CANCEL}, 2, SYSREQ_SCALEFADEVOL},

    /* TRIM */
    { 584, "Are you sure you want to trim the song? Making a backup of the song first is recommended.", {PB_SYSREQ_TRIM_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},

    /* SAMPLE */
    { 520, "Clear sample data and start sampling? While ongoing, press any key to stop.", {PB_SYSREQ_AUDIO_REC_YES, PB_SYSREQ_EXIT_NO}, 2, SYSREQ_REQUEST},

    /* ---------- ERROR/WARN/NOTICE MESSAGES (with one OK button) ----------  */
    {0, NULL, {0}, 0, 0}, /* Error Messages start indicator */

    { 296, "Invalid constant expressions.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Sample can't be minimized any further!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "General I/O error while writing to WAV (is the file in use)?", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Error: Couldn't find a working audio mode... You'll get no sound / replayer timer!", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_MESSAGE },
    { 520, "Error: Couldn't set config file location. You can't load/save the config!", { PB_SYSREQ_MIDDLE_OK_DUMMY }, 1, SYSREQ_MESSAGE },
    { 296, "Error: The filename can't be empty!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error: The very first character in the filename can't be . (dot)!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Error: The filename can't contain the following characters: \\ / : * ? \" < > |", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE}, 
    { 344, "Couldn't delete file: Access denied!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Couldn't delete directory: Access denied!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Couldn't rename file: Access denied, or file already exists!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Couldn't rename directory: Access denied, or dir already exists!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 584, "Couldn't create directory: Access denied, or a dir with the same name already exists!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Couldn't set directory path!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error: Incompatible instrument!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error: Incompatible format version!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error: You can't load sample/instrument data into instrument #0!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 420, "Couldn't open file/directory! No permission or in use?", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 420, "Couldn't open drive! Make sure there's a disk in the drive.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Note: This setting is not applied until you close and reopen the program.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Note: This setting is not properly applied until you close and reopen the program.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "You can't change the window size while in fullscreen mode!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 584, "Warning: The module contained pattern lengths above 256! They were truncated to 64.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Warning: Stereo samples were found and were converted to mono.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Warning: The instrument contained stereo samples! They were mixed to mono.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 584, "Warning: The module had over 128 instruments! Only 128 of them were loaded.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Warning: S3M channel panning is not compatible with FT2!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Linear frequency table used!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Warning: Song length is too long! (max 128 in .mod)", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Warning: Too many patterns! (max 100 in .mod)", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Warning: Too many instruments! (max 31 in .mod)", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Warning: Note(s) below A-0 was found and was limited to A-0!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Warning: Incompatible instruments found!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Warning: Incompatible pattern effects used!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Error saving as .mod: A pattern with length above/below 64 (3F) was found!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 232, "No range specified!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 232, "Set a larger range!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading .xm: Unsupported XM module version (vx.xx)!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading .xm: Incompatible amount of channels!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading .xm: The song has more than 256 patterns!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Not enough memory!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Pattern is too long to be expanded.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error opening config file for reading!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error opening config file for writing!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading config: the config file is not valid!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "General I/O error during loading! Is the file in use?", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "General I/O error during saving! Is the file in use?", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Error: Can't load a WAV as a module!",  {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading module: Not a valid XM/S3M/STM/MOD file!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "Error loading module: Either not a module or a non-supported .mod!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error loading module: Out of memory!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Cannot show empty range!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Not enough sample data outside loop!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 232, "Invalid range!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 520, "No range selected! Make a small range that includes loop start or loop end.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "X-Fade can only be used on a loop-enabled sample!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Not enough memory! (Disable \"cut to buffer\")", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading sample: The sample is not supported or is invalid!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Error loading sample: This AIFF type (AIFC) is not supported!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error saving sample: The sample is empty!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Error saving instrument: The instrument is empty!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "Error: There is no copied data to paste!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Error: No pattern data is marked!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 584, "Error: The program needs to be compiled with SDL 2.0.5 or later to support audio sampling.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Error: Couldn't open audio input device.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Error: Couldn't open that audio output device!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 440, "Procedure failed: Error creating thread! Please try again.", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Error: Help section not found in help text data!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 376, "Error: Couldn't render help text! Parsing error?", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 344, "The current pattern is empty!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
    { 296, "Error: Instrument has no samples!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},

    { 296, "This function is not implemented yet!", {PB_SYSREQ_MIDDLE_OK_DUMMY}, 1, SYSREQ_MESSAGE},
};

/* these are a list of what keyboard keys calls what function in System Requests... */
static sysReqKeys_t sysReqKeys[ERROR_MSGS_START] =
{
    /* numKeys, keys, func1, func2, func3, func4, funcEnter, funcEsc */

    /* ------ KILL REQUESTS ------ */
    { 4, {SDLK_a, SDLK_s, SDLK_i, SDLK_c}, pbZapAll, pbZapSong, pbZapInstr, hideSystemRequest, pbZapAll, hideSystemRequest },

    /* ------ TRANSPOE REQUESTS ------- */
    { 2, {SDLK_y, SDLK_n, }, doTranspose, hideSystemRequest, NULL, NULL, doTranspose, hideSystemRequest },

    /* ------ CONFIG REQUESTS ------- */
    { 2, {SDLK_y, SDLK_n}, resetConfig, hideSystemRequest, NULL, NULL, resetConfig, hideSystemRequest },

    /* ------ EXIT SYSTEM REQUESTS ------ */
    { 2, {SDLK_y, SDLK_n}, quitProgram, hideSystemRequest, NULL, NULL, quitProgram, hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, quitProgram, hideSystemRequest, NULL, NULL, quitProgram, hideSystemRequest },

    /* shrink/expand pattern */
    { 2, {SDLK_y, SDLK_n}, shrinkPattern, hideSystemRequest, NULL, NULL, shrinkPattern, hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, setNewLenAndPastePattern, pastePatternNoLenCheck, NULL, NULL, setNewLenAndPastePattern, pastePatternNoLenCheck },

    /* clear instrument */
    { 2, {SDLK_o, SDLK_c}, clearCurInstr, hideSystemRequest, NULL, NULL, clearCurInstr, hideSystemRequest },

    /* NIBBLES */
    { 2,{ SDLK_y, SDLK_n }, nibblesRestartYes, hideSystemRequest,        NULL, NULL, nibblesRestartYes,        hideSystemRequest },
    { 2,{ SDLK_y, SDLK_n }, nibblesExit2,      hideSystemRequest,        NULL, NULL, nibblesExit2,             hideSystemRequest },
    { 1,{ SDLK_o }, hideSystemRequest,         hideSystemRequest,        NULL, NULL, hideSystemRequest,        hideSystemRequest },
    { 1,{ SDLK_o }, hideSystemRequest,         hideSystemRequest,        NULL, NULL, hideSystemRequest,        hideSystemRequest },
    { 1,{ SDLK_o }, nibblesPlayerDiedOK,       nibblesPlayerDiedOK,      NULL, NULL, nibblesPlayerDiedOK,      nibblesPlayerDiedOK },
    { 1,{ SDLK_o }, nibblesPlayerDiedOK,       nibblesPlayerDiedOK,      NULL, NULL, nibblesPlayerDiedOK,      nibblesPlayerDiedOK },
    { 1,{ SDLK_o }, nibblesPlayerDiedOK,       nibblesPlayerDiedOK,      NULL, NULL, nibblesPlayerDiedOK,      nibblesPlayerDiedOK }, 
    { 1,{ SDLK_o }, nibblesGameOverOK,         nibblesGameOverOK,        NULL, NULL, nibblesGameOverOK,        nibblesGameOverOK },
    { 1,{ SDLK_o }, nibblesLevelFinishedOK,    nibblesLevelFinishedOK,   NULL, NULL, nibblesLevelFinishedOK,   nibblesLevelFinishedOK },
    { 1,{ SDLK_o }, nibblesPlayer1NameOK,      nibblesPlayer1NameOK,     NULL, NULL, nibblesPlayer1NameOK,     nibblesPlayer1NameOK },
    { 1,{ SDLK_o }, nibblesPlayer2NameOK,      nibblesPlayer2NameOK,     NULL, NULL, nibblesPlayer2NameOK,     nibblesPlayer2NameOK },
    { 1,{ SDLK_o }, hideSystemRequest,         hideSystemRequest,        NULL, NULL, hideSystemRequest,        hideSystemRequest },
    { 1,{ SDLK_o }, hideSystemRequest,         hideSystemRequest,        NULL, NULL, hideSystemRequest,        hideSystemRequest },
    { 1,{ SDLK_o }, hideSystemRequest,         hideSystemRequest,        NULL, NULL, hideSystemRequest,        hideSystemRequest },

    /* SAMPLE EDITOR */
    { 2, {SDLK_o, SDLK_c}, clearCurSample,    hideSystemRequest, NULL, NULL, clearCurSample,    hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, convSampleTo8Bit,  hideSystemRequest, NULL, NULL, convSampleTo8Bit,  hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, convSampleTo16Bit, hideSystemRequest, NULL, NULL, convSampleTo16Bit, hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, minimizeSample,    hideSystemRequest, NULL, NULL, minimizeSample,    hideSystemRequest },
    { 4, {SDLK_l, SDLK_r, SDLK_c, SDLK_a}, stereoSampleReadLeft, stereoSampleReadRight, stereoSampleConvert, hideSystemRequest, stereoSampleConvert, hideSystemRequest },
    { 3, {SDLK_a, SDLK_g, SDLK_e}, sampleChangeVolume, sampleGetMaxVolume, hideSystemRequest, NULL, sampleChangeVolume,  hideSystemRequest },
    { 2, {SDLK_a, SDLK_e}, resampleSample, hideSystemRequest, NULL, NULL, resampleSample, hideSystemRequest },
    { 2, {SDLK_a, SDLK_e}, mixSample, hideSystemRequest, NULL, NULL, mixSample, hideSystemRequest },
    { 2, {SDLK_a, SDLK_e}, createEcho, hideSystemRequest, NULL, NULL, createEcho, hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, saveRange2, hideSystemRequest, NULL, NULL, saveRange2, hideSystemRequest },

    /* DISK OP */
    { 2, {SDLK_y, SDLK_n}, diskOpDelete,       hideSystemRequest,     NULL, NULL, diskOpDelete,       hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, diskOpRenameAnsi,   hideSystemRequest,     NULL, NULL, diskOpRenameAnsi,   hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, diskOpRenameAnsi,   hideSystemRequest,     NULL, NULL, diskOpRenameAnsi,   hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, diskOpMakeDirAnsi,  hideSystemRequest,     NULL, NULL, diskOpMakeDirAnsi,  hideSystemRequest },
    { 2, {SDLK_o, SDLK_c}, diskOpSetPathAnsi,  hideSystemRequest,     NULL, NULL, diskOpSetPathAnsi,  hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, diskOpSave2,        hideSystemRequest,     NULL, NULL, diskOpSave2,        hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, wavRenderOverwrite, hideSystemRequest,     NULL, NULL, wavRenderOverwrite, hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, openFile2,          hideSystemRequest,     NULL, NULL, openFile2,          hideSystemRequest },
    { 2, {SDLK_y, SDLK_n}, loadDroppedFile2,   cancelLoadDroppedFile, NULL, NULL, loadDroppedFile2,   cancelLoadDroppedFile },

    /* scale fade volume */
    { 2, {SDLK_o, SDLK_c}, handleScaleFadeVolume, cancelScaleFadeVolume, NULL, NULL, handleScaleFadeVolume, cancelScaleFadeVolume },

    /* trim */
    { 2, {SDLK_y, SDLK_n}, trim, hideSystemRequest, NULL, NULL, trim, hideSystemRequest },

    /* sampling */
    { 2, {SDLK_y, SDLK_n}, srStartSampling, hideSystemRequest, NULL, NULL, srStartSampling, hideSystemRequest }
};

static void drawSysReqFeatures(sysReq_t *sysReq, uint16_t x)
{
    char *windowTitle;
    uint8_t i;
    int16_t textBoxID;
    const uint16_t y = SYSTEM_REQUEST_Y;
    int32_t textX;
    textBox_t *t;

    textBoxID   = -1; /* no text box present so far */
    windowTitle = NULL;

    switch (sysReq->type)
    {
        default:
        case SYSREQ_REQUEST:  windowTitle = "System request";               break;
        case SYSREQ_MESSAGE:  windowTitle = "System message";               break;
        case SYSREQ_NIBBLES:  windowTitle = "Nibbles message";              break;
        case SYSREQ_NIBBLES3: windowTitle = "Triton productions declares:"; break;

        case SYSREQ_NIBBLES1:
        {
            windowTitle = "Player 1 - Enter your name:";
            textBoxID   = TB_NIB_PLAYER1_NAME;
        }
        break;

        case SYSREQ_NIBBLES2:
        {
            windowTitle = "Player 2 - Enter your name:";
            textBoxID   = TB_NIB_PLAYER2_NAME;
        }
        break;

        case SYSREQ_RENAMEFILE:
        {
            windowTitle = "Enter new filename:";
            textBoxID   = TB_DISKOP_RENAME_NAME;
        }
        break;

        case SYSREQ_RENAMEDIR:
        {
            windowTitle = "Enter new directoryname:";
            textBoxID   = TB_DISKOP_RENAME_NAME;
        }
        break;

        case SYSREQ_MAKEDIR:
        {
            windowTitle = "Enter directoryname:";
            textBoxID   = TB_DISKOP_MAKEDIR_NAME;
        }
        break;

        case SYSREQ_SETPATH:
        {
            windowTitle = "Enter new directory path:";
            textBoxID   = TB_DISKOP_SETPATH_NAME;
        }
        break;

        case SYSREQ_SAVERANGE:
        {
            windowTitle = "Enter filename:";
            textBoxID   = TB_SAVE_RANGE_FILENAME;
        }
        break;

        case SYSREQ_SCALEFADEVOL:
        {
                 if (editor.scaleFadeVolumeMode == 0) windowTitle = "Volume scale-fade track (start-, end scale)";
            else if (editor.scaleFadeVolumeMode == 1) windowTitle = "Volume scale-fade pattern (start-, end scale)";
            else if (editor.scaleFadeVolumeMode == 2) windowTitle = "Volume scale-fade block (start-, end scale)";

            textBoxID = TB_SCALE_FADE_VOL;
        }
        break;
    }

    if (textBoxID >= 0)
    {
        MY_ASSERT(textBoxID < NUM_TEXTBOXES)

        textBoxes[textBoxID].visible = true;
        mouse.lastEditBox = textBoxID;
        editor.ui.editTextFlag  = true;
    }

    MY_ASSERT(windowTitle != NULL)

    /* draw window title text */
    textX = (SCREEN_W / 2) - (getTextWidth(windowTitle, FONT_TYPE1) / 2);
    textOutShadow((uint16_t)(textX), y + 4, PAL_BUTTON1, PAL_BUTTON2, windowTitle);

    /* render text */
    if (sysReq->text != NULL)
    {
        textX = (SCREEN_W / 2) - (getTextWidth(sysReq->text, FONT_TYPE1) / 2);
        textOutShadow((uint16_t)(textX), y + 24, PAL_BUTTON1, PAL_BUTTON2, sysReq->text);
    }

    /* render "Don't show again" checkboxes and texts for some system requests... */
    switch (editor.ui.systemRequestID)
    {
        case SR_S3M_LOADED:
        {
            checkBoxes[CB_SR_S3M_DONT_SHOW].x = x + 5;
            showCheckBox(CB_SR_S3M_DONT_SHOW);
            textOutShadow(x + 21, 301, PAL_BUTTON1, PAL_BUTTON2, "Don't show again");
        }
        break;

        case SR_SETTING_NOT_APPLIED_YET:
        {
            checkBoxes[CB_SR_NOT_YET_APPLIED_DONT_SHOW].x = x + 5;
            showCheckBox(CB_SR_NOT_YET_APPLIED_DONT_SHOW);
            textOutShadow(x + 21, 301, PAL_BUTTON1, PAL_BUTTON2, "Don't show again");
        }
        break;

        default: break;
    }

    /* draw text box + frame (if needed) */
    if ((textBoxID >= 0) && (textBoxID < NUM_TEXTBOXES))
    {
        t = &textBoxes[textBoxID];

        clearRect(t->x, t->y, t->w, t->h);

        hLine(t->x - 1,    t->y - 1,    t->w + 2, PAL_BUTTON2);
        vLine(t->x - 1,    t->y,        t->h + 1, PAL_BUTTON2);
        hLine(t->x,        t->y + t->h, t->w + 1, PAL_BUTTON1);
        vLine(t->x + t->w, t->y,        t->h,     PAL_BUTTON1);

        drawTextBox(textBoxID);
    }

    /* show/activate push buttons... */
    for (i = 0; i < sysReq->numButtons; ++i)
        showPushButton(sysReq->buttonIDs[i]);
}

void drawSystemRequest(void)
{
    uint16_t x, w;
    const uint16_t y = SYSTEM_REQUEST_Y;
    const uint16_t h = SYSTEM_REQUEST_H;
    sysReq_t *sysReq;

    if (queueLen == 0)
        return;

    editor.ui.systemRequestShown = true;
    editor.ui.systemRequestID = queue[queuePos];

    if (editor.ui.systemRequestID == SR_SAMP_VOLUME)
    {
        drawSampleVolumeBox();
        return;
    }
    else if (editor.ui.systemRequestID == SR_SAMP_RESAMPLE)
    {
        drawResampleBox();
        return;
    }
    else if (editor.ui.systemRequestID == SR_SAMP_MIX_SAMPLE)
    {
        drawMixSampleBox();
        return;
    }
    else if (editor.ui.systemRequestID == SR_SAMP_ECHO)
    {
        drawEchoBox();
        return;
    }

    sysReq = &sysReqs[editor.ui.systemRequestID];

    w = sysReq->w;

    MY_ASSERT((w / 2) <= (SCREEN_W / 2))

    x = (SCREEN_W / 2) - (w / 2);

    /* main fill */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* outer border */
    vLine(x,         y,         h - 1, PAL_BUTTON1);
    hLine(x + 1,     y,         w - 2, PAL_BUTTON1);
    vLine(x + w - 1, y,         h,     PAL_BUTTON2);
    hLine(x,         y + h - 1, w - 1, PAL_BUTTON2);

    /* inner border */
    vLine(x + 2,     y + 2,     h - 5, PAL_BUTTON2);
    hLine(x + 3,     y + 2,     w - 6, PAL_BUTTON2);
    vLine(x + w - 3, y + 2,     h - 4, PAL_BUTTON1);
    hLine(x + 2,     y + h - 3, w - 4, PAL_BUTTON1);

    /* title bottom line */
    hLine(x + 3, y + 16, w - 6, PAL_BUTTON2);
    hLine(x + 3, y + 17, w - 6, PAL_BUTTON1);

    drawSysReqFeatures(sysReq, x);
}

void hideSystemRequest(void)
{
    uint16_t i;
    sysReq_t *sysReq;

    exitTextEditing();

    /* hide check boxes for special system requests */
    hideCheckBox(CB_SR_S3M_DONT_SHOW);
    hideCheckBox(CB_SR_NOT_YET_APPLIED_DONT_SHOW);
    hideCheckBox(CB_SAMP_ECHO_ADD_MEMORY);

    /* hide scroll bars for special system requests */
    hideScrollBar(SB_SAMPVOL_START);
    hideScrollBar(SB_SAMPVOL_END);
    hideScrollBar(SB_RESAMPLE_HTONES);
    hideScrollBar(SB_MIX_BALANCE);
    hideScrollBar(SB_ECHO_NUM);
    hideScrollBar(SB_ECHO_DISTANCE);
    hideScrollBar(SB_ECHO_FADEOUT);

    /* hide text boxes for special system requests */
    hideTextBox(TB_DISKOP_MAKEDIR_NAME);
    hideTextBox(TB_DISKOP_RENAME_NAME);
    hideTextBox(TB_DISKOP_SETPATH_NAME);
    hideTextBox(TB_SAVE_RANGE_FILENAME);
    hideTextBox(TB_NIB_PLAYER1_NAME);
    hideTextBox(TB_NIB_PLAYER2_NAME);
    hideTextBox(TB_SCALE_FADE_VOL);

    if ((queueLen > 0) && (queuePos < queueLen))
    {
        sysReq = &sysReqs[queue[queuePos++]];
        for (i = 0; i < sysReq->numButtons; ++i)
            hidePushButton(sysReq->buttonIDs[i]);

        if ((editor.ui.systemRequestID == SR_EXIT_SONG_MODIFIED) || (editor.ui.systemRequestID == SR_EXIT))
            editor.ui.exitSysReqOpen = false;

        showBottomScreen();

        if (queuePos >= queueLen)
        {
            queuePos = 0;
            queueLen = 0;

            memset(queue, 0, sizeof (queue));
        }
    }

    editor.ui.systemRequestShown = false;
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
}

void sysReqQueue(uint16_t sysReqID)
{
    uint16_t i;

    MY_ASSERT(sysReqID < NUM_SYSREQS)

    if (queueLen >= MAX_SYSREQ_QUEUE_LEN)
        return;

    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);

    /* release possibly stuck (pressed) GUI elements */
    if (mouse.lastUsedObjectID != OBJECT_ID_NONE)
    {
        mouse.lastUsedObjectID   = OBJECT_ID_NONE;
        mouse.lastUsedObjectType = OBJECT_NONE;

        for (i = 0; i < NUM_RADIOBUTTONS; ++i)
        {
            if (radioButtons[i].state == RADIOBUTTON_PRESSED)
            {
                radioButtons[i].state = RADIOBUTTON_UNCHECKED;
                if (radioButtons[i].visible)
                    drawRadioButton(i);
            }
        }

        for (i = 0; i < NUM_CHECKBOXES; ++i)
        {
            if (checkBoxes[i].state == CHECKBOX_PRESSED)
            {
                checkBoxes[i].state = CHECKBOX_UNPRESSED;
                if (checkBoxes[i].visible)
                    drawCheckBox(i);
            }
        }

        for (i = 0; i < NUM_PUSHBUTTONS; ++i)
        {
            if (pushButtons[i].state == PUSHBUTTON_PRESSED)
            {
                pushButtons[i].state = PUSHBUTTON_UNPRESSED;
                if (pushButtons[i].visible)
                    drawPushButton(i);
            }
        }

        for (i = 0; i < NUM_SCROLLBARS; ++i)
        {
            scrollBars[i].state = SCROLLBAR_UNPRESSED;
            if (scrollBars[i].visible)
                drawScrollBar(i);
        }
    }

    queue[queueLen++] = sysReqID;
    editor.ui.systemRequestShown = true;
}

void checkSysReqKeys(SDL_Keycode keycode)
{
    uint16_t i;
    sysReqKeys_t *sysReqKey;

    keyb.ignoreCurrKeyUp = true;

    /* HANDLE ERROR MESSAGE KEYS */
    if (editor.ui.systemRequestID > ERROR_MSGS_START)
    {
        if ((keycode == SDLK_RETURN) || (keycode == SDLK_KP_ENTER) || (keycode == SDLK_ESCAPE) || (keycode == SDLK_o))
        {
            if (keycode == SDLK_o) /* kludge */
                keyb.ignoreTextEditKey = true;

            hideSystemRequest(); /* warning message keys can only close the box, nothing else */
        }

        return;
    }

    /* HANDLE NORMAL MESSAGE KEYS */

    MY_ASSERT(editor.ui.systemRequestID < NUM_SYSREQS)

    sysReqKey = &sysReqKeys[editor.ui.systemRequestID];

    /* test return and escape */
    if ((keycode == SDLK_RETURN) || (keycode == SDLK_KP_ENTER))
    {
        if (sysReqKey->callbackFuncEnter != NULL)
            sysReqKey->callbackFuncEnter();

        return;
    }
    else if (keycode == SDLK_ESCAPE)
    {
        if (sysReqKey->callbackFuncEsc != NULL)
            sysReqKey->callbackFuncEsc();

        return;
    }

    /* check for custom sysreq keys */
    for (i = 0; i < sysReqKey->numKeys; ++i)
    {
        if (keycode == sysReqKey->keys[i])
        {
            switch (i)
            {
                default:
                case 0: if (sysReqKey->callbackFunc1 != NULL) sysReqKey->callbackFunc1(); break;
                case 1: if (sysReqKey->callbackFunc2 != NULL) sysReqKey->callbackFunc2(); break;
                case 2: if (sysReqKey->callbackFunc3 != NULL) sysReqKey->callbackFunc3(); break;
                case 3: if (sysReqKey->callbackFunc4 != NULL) sysReqKey->callbackFunc4(); break;
            }

            break; /* key found, break loop */
        }
    }
}
