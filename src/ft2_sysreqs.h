#ifndef __FT2_SYSREQS_H
#define __FT2_SYSREQS_H

#include <stdint.h>

enum /* SYSREQS */
{
    SR_WAV_OVERWRITE,

    /* ERROR/NOTICE/WARN MESSAGES (with OK button) */
    ERROR_MSGS_START,

    SR_SAMP_MINIMIZE_NOT_NEEDED,
    SR_WAV_WRITE_ERROR,
    SR_AUDIO_MODE_ERROR,
    SR_CONFIGPATH_ERROR,
    SR_EMPTY_FILENAME,
    SR_ILLEGAL_FILENAME_DOT,
    SR_ILLEGAL_FILENAME,
    SR_MAKE_DIR_ERROR,
    SR_SET_PATH_ERROR,
    SR_LOAD_INCOMPAT_INSTR,
    SR_LOAD_WRONG_VERSION,
    SR_LOAD_INSTR0_ERROR,
    SR_SETTING_NOT_APPLIED_YET,
    SR_FILTERING_NOT_APPLIED_YET,
    SR_CANT_CHANGE_SETTING_FULLSCREEN,
    SR_OVERFLOWN_PATT,
    SR_STERO_SAMPLES,
    SR_INSTR_STEREO_SAMPLES,
    SR_XM_OVER_128_INS,
    SR_S3M_LOADED,
    SR_SAVE_LINEAR_FREQ,
    SR_SAVE_WARN_SONGLEN,
    SR_SAVE_WARN_PATTS,
    SR_SAVE_WARN_INSTR,
    SR_SAVE_NOTE_UNDERFLOW,
    SR_SAVE_INCOMPAT_INSTR,
    SR_SAVE_INCOMPAT_EFX,
    SR_SAVE_ERR_PATTLEN,
    SR_NO_RANGE,
    SR_SET_LAGER_RANGE,
    SR_UNSUPPORTED_XM_VER,
    SR_UNSUPPORTED_CHS,
    SR_UNSUPPORTED_PATTS,
    SR_OOM_ERROR,
    SR_EXPAND_PATT_ERROR,
    SR_CONFIG_CANT_LOAD,
    SR_CONFIG_CANT_SAVE,
    SR_CONFIG_NOT_VALID,
    SR_LOAD_IO_ERROR,
    SR_SAVE_IO_ERROR,
    SR_LOAD_WAV_SONG_ERROR,
    SR_LOAD_MOD_ERROR,
    SR_LOAD_MOD_ERROR2,
    SR_LOAD_MOD_OOM,
    SR_SAMP_SR_ERROR,
    SR_XFADE_ERROR_1,
    SR_XFADE_ERROR_2,
    SR_XFADE_ERROR_3,
    SR_XFADE_ERROR_4,
    SR_CUT_TO_BUF_OOM,
    SR_SAMP_LOAD_ERROR,
    SR_SAMP_LOAD_AIFC_ERROR,
    SR_SAMP_SAVE_EMPTY,
    SR_INST_SAVE_EMPTY,
    SR_AUD_OUT_DEV_ERROR,
    SR_THREAD_ERROR,
    SR_HELP_SUBJECT_NOT_FOUND,
    SR_HELP_RENDER_ERROR,
    SR_CURR_PATT_EMPTY,
    SR_INSTR_HAS_NO_SMPS,

    SR_NOT_IMPLEMENTED,
    NUM_SYSREQS
};

enum
{
    SYSREQ_REQUEST      =  0,
    SYSREQ_MESSAGE      =  1,
    SYSREQ_NIBBLES      =  2,
    SYSREQ_NIBBLES1     =  3,
    SYSREQ_NIBBLES2     =  4,
    SYSREQ_NIBBLES3     =  5,
    SYSREQ_RENAMEFILE   =  6,
    SYSREQ_RENAMEDIR    =  7,
    SYSREQ_MAKEDIR      =  8,
    SYSREQ_SETPATH      =  9,
    SYSREQ_SCALEFADEVOL = 10,
    SYSREQ_SAMPLEVOLUME = 11,
    SYSREQ_SAVERANGE    = 12,
};

int16_t okBoxThreadSafe(int16_t typ, char *headline, char *text);
int16_t okBox(int16_t typ, char *headline, char *text);
int16_t quitBox(uint8_t skipQuitMsg);
int16_t inputBox(int16_t typ, char *headline, char *edText, uint16_t maxStrLen);

/* for thread-safe version of okBox() */
struct
{
    volatile uint8_t active;
    int16_t typ, returnData;
    char *headline, *text;
} okBoxData;

#endif
