#ifndef __FT2_AUDIO_H
#define __FT2_AUDIO_H

#include <stdint.h>
#include <SDL2/SDL.h>
#include "ft2_replayer.h"

enum
{
    FREQ_TABLE_LINEAR = 0,
    FREQ_TABLE_AMIGA  = 1,
};

/* for audio/video sync queue (must be 2^n-1 - don't mess with this! It's big enough as is.) */
#define SYNC_QUEUE_LEN 2047

#define MAX_AUDIO_DEVICES 99

#define MIN_AUDIO_FREQ 44100
#define MAX_AUDIO_FREQ 96000
#define MAX_SAMPLES_PER_TICK (((MAX_AUDIO_FREQ * 5) / 2) / MIN_BPM)

struct audio_t
{
    volatile int8_t locked;
    char *currInputDevice, *currOutputDevice, *lastWorkingAudioDeviceName;
    char *inputDeviceNames[MAX_AUDIO_DEVICES], *outputDeviceNames[MAX_AUDIO_DEVICES];
    int8_t freqTable, volumeRampingFlag, interpolationFlag, rescanAudioDevicesSupported;
    int32_t inputDeviceNum, outputDeviceNum, lastWorkingAudioFreq, lastWorkingAudioBits;
    int32_t quickVolSizeVal, *mixBufferL, *mixBufferR;
    uint32_t freq, scopeFreqMul;
    uint64_t tickTime64;
    SDL_AudioDeviceID dev;
} audio;

typedef struct
{
    const int8_t *sampleData8;
    const int16_t *sampleData16;
    uint8_t backwards, SVol, SPan, isFadeOutVoice;
    int32_t SLVol1, SRVol1, SLVol2, SRVol2, SLVolIP, SRVolIP, SVolIPLen, SPos, SLen, SRepS, SRepL;
    uint32_t SPosDec, SFrq;
    void (*mixRoutine)(void *, int32_t); /* function pointer to mix routine */
} voice_t;

typedef struct pattSyncData_t
{
    /* for pattern editor */
    int16_t pattern, patternPos, globalVol, songPos;
    uint16_t timer, speed, tempo;
    uint64_t timestamp;
} pattSyncData_t;

struct pattSync
{
    int32_t readPos, writePos;
    pattSyncData_t data[SYNC_QUEUE_LEN + 1];
} pattSync;

typedef struct chSyncData_t
{
    channel_t channels[MAX_VOICES];
    uint64_t timestamp;
} chSyncData_t;

struct chSync
{
    int32_t readPos, writePos;
    chSyncData_t data[SYNC_QUEUE_LEN + 1];
} chSync;

int32_t pattQueueReadSize(void);
int32_t pattQueueWriteSize(void);
int8_t pattQueuePush(pattSyncData_t t);
int8_t pattQueuePop(void);
pattSyncData_t *pattQueuePeek(void);
uint64_t getPattQueueTimestamp(void);
int32_t chQueueReadSize(void);
int32_t chQueueWriteSize(void);
int8_t chQueuePush(chSyncData_t t);
int8_t chQueuePop(void);
chSyncData_t *chQueuePeek(void);
uint64_t getChQueueTimestamp(void);

uint32_t getVoiceRate(uint8_t i);
void setAudioAmp(int16_t ampFactor, int16_t master, uint8_t bitDepth32Flag);
void setNewAudioFreq(uint32_t freq);
void setBackOldAudioFreq(void);
void setSpeed(uint16_t bpm);
void audioSetVolRamp(uint8_t volRamp);
void audioSetInterpolation(uint8_t interpolation);
void stopVoice(uint8_t i);
int8_t setupAudio(int8_t showErrorMsg);
void closeAudio(void);
void pauseAudio(void);
void resumeAudio(void);
int8_t setNewAudioSettings(void);
uint8_t setupAudioBuffers(void);
void freeAudioBuffers(void);
void resetDitherSeed(void);
void lockAudio(void);
void unlockAudio(void);
void lockMixerCallback(void);
void unlockMixerCallback(void);
void updateSendAudSamplesRoutine(uint8_t lockMixer);
void mix_SaveIPVolumes(void);
void mix_UpdateChannelVolPanFrq(void);
uint32_t mixReplayerTickToBuffer(uint8_t *stream, uint8_t bitDepth);
//void benchmarkAudioChannelMixer(void); // for development testing

pattSyncData_t *pattSyncEntry;
chSyncData_t *chSyncEntry;

#endif
