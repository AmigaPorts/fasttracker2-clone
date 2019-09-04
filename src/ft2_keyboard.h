#ifndef __FT2_KEYBOARD_H
#define __FT2_KEYBOARD_H

#include <stdint.h>
#include <SDL2/SDL.h>

struct keyb_t
{
    uint8_t ignoreCurrKeyUp, ignoreTextEditKey, numPadPlusPressed, keyRepeat, keyPressed;
    uint32_t keyModifierDown, commandPressed, leftCommandPressed;
    uint32_t leftShiftPressed, leftCtrlPressed, ctrlPressed, leftAltPressed;
} keyb;

int8_t scancodeKeyToNote(SDL_Scancode skey);
void keyUpHandler(SDL_Scancode scancode, SDL_Keycode keycode);
void keyDownHandler(SDL_Scancode scancode, SDL_Keycode keycode, uint8_t keyWasRepeated);
void readKeyModifiers(void);

#endif
