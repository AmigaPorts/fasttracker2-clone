#ifndef __FT2_HELP_H
#define __FT2_HELP_H

#include <stdint.h>

#define HELP_LINE_HEIGHT 11
#define HELP_WINDOW_HEIGHT 164
#define HELP_WINDOW_LINES 15
#define HELP_TEXT_BUFFER_W 472

void helpScrollUp(void);
void helpScrollDown(void);
void helpScrollSetPos(int32_t pos);

void showHelpScreen(void);
void hideHelpScreen(void);
void exitHelpScreen(void);

void freeHelpTextBuffer(void);

/* RADIO BUTTONS */

void rbHelpFeatures(void);
void rbHelpEffects(void);
void rbHelpKeyboard(void);
void rbHelpHowToUseFT2(void);
void rbHelpFAQ(void);
void rbHelpKnownBugs(void);

#endif
