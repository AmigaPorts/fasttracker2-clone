#ifndef __FT2_MIDI_H
#define __FT2_MIDI_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define MIDI_INPUT_SELECTOR_BOX_WIDTH 247
#define MAX_MIDI_DEVICES 99

struct midi_t
{
    char *inputDeviceName, *inputDeviceNames[MAX_MIDI_DEVICES];
    volatile uint8_t closeMidiOnExit, initThreadDone;
    uint32_t inputDevice;
    uint8_t enable, rescanDevicesFlag;
    int16_t currMIDIVibDepth, currMIDIPitch;
    int32_t numInputDevices;
} midi;

void closeMidiInDevice(void);
void freeMidiIn(void);
uint8_t initMidiIn(void);
uint8_t openMidiInDevice(uint32_t deviceID);

void recordMIDIEffect(uint8_t effTyp, uint8_t effData);

uint8_t saveMidiInputDeviceToConfig(void);
uint8_t setMidiInputDeviceFromConfig(void);
void freeMidiInputDeviceList(void);
void rescanMidiInputDevices(void);
void drawMidiInputList(void);
void scrollMidiInputDevListUp(void);
void scrollMidiInputDevListDown(void);
void sbMidiInputSetPos(uint32_t pos);
uint8_t testMidiInputDeviceListMouseDown(void);

int32_t SDLCALL initMidiFunc(void *ptr);

#endif
