#ifndef __FT2_EVENTS_H
#define __FT2_EVENTS_H

#ifdef _WIN32
#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#endif
#include <stdint.h>

enum
{
    EVENT_NONE                = 0,
    EVENT_LOADMUSIC_ARGV      = 1,
    EVENT_LOADMUSIC_DRAGNDROP = 2,
    EVENT_LOADMUSIC_DISKOP    = 3,
};

void handleEvents(void);
void handleInput(void);
void setupCrashHandler(void);

#ifdef _WIN32
int8_t handleSingleInstancing(int32_t argc, char **argv);
void closeSingleInstancing(void);
#else
#endif

#endif
