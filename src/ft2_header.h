#ifndef __FT2_HEADER_H
#define __FT2_HEADER_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <emmintrin.h> /* for intrinsics */
#else
#include <limits.h> /* PATH_MAX */
#endif
#include "ft2_replayer.h"

#define BETA_VERSION 135

/* do NOT change these! It will only mess things up... */
#define VBLANK_HZ 60
#define SCREEN_W 632
#define SCREEN_H 400

#ifndef _WIN32
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define DIR_DELIMITER '/'
#else
#define DIR_DELIMITER '\\'
#define PATH_MAX 260
#endif

/* some of these may not be platform safe... */
#define SGN(x) (((x) >= 0) ? 1 : -1)
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* fast 32-bit -> 8-bit clamp */
#define CLAMP8(i) if ((int8_t)(i) != i) i = 0x7F ^ (i >> 31)

/* fast 32-bit -> 16-bit clamp */
#define CLAMP16(i) if ((int16_t)(i) != i) i = 0x7FFF ^ (i >> 31)

#define SWAP16(value) \
( \
    (((uint16_t)((value) & 0x00FF)) << 8) | \
    (((uint16_t)((value) & 0xFF00)) >> 8)   \
)

#define SWAP32(value) \
( \
    (((uint32_t)((value) & 0x000000FF)) << 24) | \
    (((uint32_t)((value) & 0x0000FF00)) <<  8) | \
    (((uint32_t)((value) & 0x00FF0000)) >>  8) | \
    (((uint32_t)((value) & 0xFF000000)) >> 24)   \
)

/* round and convert double/float to int32_t */
#if defined __APPLE__ || defined _WIN32 || defined __i386__ || defined __amd64__
#define double2int32_round(i, d) (i = _mm_cvtsd_si32(_mm_load_sd(&d))) /* SSE2 */
#define float2int32_round(i, f)  (i = _mm_cvt_ss2si(_mm_load_ss(&f)))  /* SSE  */
#else
#define double2int32_round(i, d) (i = (int32_t)(round(d)))
#define float2int32_round(i, f)  (i = (int32_t)(roundf(f)))
#endif

struct editor_t
{
    struct ui_t
    {
        volatile uint8_t setMouseBusy, setMouseIdle;
        char fullscreenButtonText[24];
        int8_t buttonContrast, desktopContrast;

        /* all screens */
        uint8_t extended, sysReqShown;

        /* top screens */
        uint8_t instrSwitcherShown, aboutScreenShown, helpScreenShown, configScreenShown;
        uint8_t scopesShown, diskOpShown, nibblesShown, transposeShown, instEditorExtShown;
        uint8_t sampleEditorExtShown, advEditShown, wavRendererShown, trimScreenShown, oldTopLeftScreen;
        uint8_t drawBPMFlag, drawSpeedFlag, drawGlobVolFlag, drawPosEdFlag, drawPattNumLenFlag;
        uint8_t updatePosSections;

        /* bottom screens */
        uint8_t patternEditorShown, instEditorShown, sampleEditorShown;
        uint8_t channelOffset, numChannelsShown, pattChanScrollShown;
        uint8_t leftLoopPinMoving, rightLoopPinMoving, recordBoxShown;
        uint8_t drawReplayerPianoFlag, drawPianoFlag, updatePatternEditor, maxVisibleChannels;
        uint16_t patternChannelWidth;
        int32_t sampleDataOrLoopDrag;

        /* backup flag for when entering/exiting extended pattern editor (TODO: this is lame and shouldn't be hardcoded) */
        uint8_t _aboutScreenShown, _helpScreenShown, _configScreenShown, _diskOpShown;
        uint8_t _nibblesShown, _transposeShown, _instEditorShown;
        uint8_t _instEditorExtShown,  _sampleEditorExtShown, _patternEditorShown;
        uint8_t _sampleEditorShown, _advEditShown, _wavRendererShown, _trimScreenShown;
        /* ------------------------------------------- */
    } ui;

    struct cursor_t
    {
        uint8_t ch;
        int8_t object;
    } cursor;

    UNICHAR *tmpFilenameU, *tmpInstrFilenameU; /* used by saving/loading threads */
    UNICHAR *configFileLocation, *audioDevConfigFileLocation, *midiConfigFileLocation;

    volatile uint8_t busy, loadMusicEvent, scopeThreadMutex, programRunning, wavIsRendering, wavReachedEndFlag;
    volatile uint8_t updateLoadedSample, updateLoadedInstrument;
    volatile FILE *wavRendererFileHandle;

    uint8_t autoPlayOnDrop, trimThreadWasDone, curSmpChannel, throwExit, editTextFlag;
    uint8_t currPanEnvPoint, currVolEnvPoint, currPaletteEdit;
    uint8_t copyMaskEnable, copyMask[5], pasteMask[5], transpMask[5], updateWindowTitle;
    uint8_t smpEd_NoteNr, instrBankSwapped, instrBankOffset, sampleBankOffset, channelMute[MAX_VOICES];
    uint8_t srcInstr, curInstr, srcSmp, curSmp, currHelpScreen, currConfigScreen, textCursorBlinkCounter, diskOpReadOnOpen;
    uint8_t keyOnTab[MAX_VOICES], diskOpReadDir, diskOpReadDone, ID_Add, curOctave;
    uint8_t sampleSaveMode, moduleSaveMode, samplingAudioFlag, NI_Play, ptnJumpPos[4];
    int16_t globalVol, songPos, pattPos;
    uint16_t tmpPattern, editPattern, speed, tempo, timer, ptnCursorY;
    int32_t samplePlayOffset, keyOffNr, keyOffTime[MAX_VOICES];
    uint32_t framesPassed, *currPaletteEntry, wavRendererTime;
    double dPerfFreq, dPerfFreqMulMicro;

    tonTyp *blkCopyBuff, *ptnCopyBuff, *trackCopyBuff, clearNote;
} editor;

#endif
