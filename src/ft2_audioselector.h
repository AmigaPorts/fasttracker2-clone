#ifndef __FT2_AUDIOSELECTOR_H
#define __FT2_AUDIOSELECTOR_H

#include <stdint.h>

#define AUDIO_SELECTORS_BOX_WIDTH 247

void setToDefaultAudioOutputDevice(void);
void setToDefaultAudioInputDevice(void);
char *getAudioOutputDeviceFromConfig(void);
char *getAudioInputDeviceFromConfig(void);
uint8_t saveAudioDevicesToConfig(const char *inputString, const char *outputString);
uint8_t testAudioDeviceListsMouseDown(void);
void rescanAudioDevices(void);
void scrollAudInputDevListUp(void);
void scrollAudInputDevListDown(void);
void scrollAudOutputDevListUp(void);
void scrollAudOutputDevListDown(void);
void sbAudOutputSetPos(int32_t pos);
void sbAudInputSetPos(int32_t pos);
void freeAudioDeviceLists(void);
void freeAudioDeviceSelectorBuffers(void);

#endif
