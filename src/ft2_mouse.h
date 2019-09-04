#ifndef __FT2_MOUSE_H
#define __FT2_MOUSE_H

#include <stdint.h>

enum
{
    MOUSE_MODE_NORMAL = 0,
    MOUSE_MODE_DELETE = 1,
    MOUSE_MODE_RENAME = 2,

    MOUSE_WHEEL_DOWN = 0,
    MOUSE_WHEEL_UP   = 1
};

struct mouse_t
{
    int8_t leftButtonPressed, rightButtonPressed, leftButtonReleased, rightButtonReleased;
    int8_t firstTimePressingButton, buttonCounter, mode, mouseOverTextBox;
    int16_t lastUsedObjectID, lastUsedObjectType, lastEditBox, x, y, lastX, lastY, xBias, yBias;
    int32_t lastScrollX, lastScrollXTmp, lastScrollY, saveMouseX, saveMouseY;
} mouse;

/* do not change these! */
#define MOUSE_CURSOR_W 26
#define MOUSE_CURSOR_H 23
#define MOUSE_GLASS_ANI_FRAMES 22
#define MOUSE_CLOCK_ANI_FRAMES 5

void setMouseShape(int16_t shape);
void setMouseMode(uint8_t mode);
void mouseWheelHandler(uint8_t directionUp);
void mouseButtonUpHandler(uint8_t mouseButton);
void mouseButtonDownHandler(uint8_t mouseButton);
void updateMouseScaling(void);
void setMouseBusy(int8_t busy); /* can be called from other threads */
void mouseAnimOn(void);
void mouseAnimOff(void);
void animateBusyMouse(void);
void handleLastGUIObjectDown(void);
void readMouseXY(void);
void resetMouseBusyAnimation(void);

#endif
