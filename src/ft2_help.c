/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_help.h"
#include "ft2_pattern_ed.h"
#include "ft2_gfxdata.h"
#include "helpdata/ft2_help_data.h"

enum
{
    HELP_FEATURES       = 0,
    HELP_EFFECTS        = 1,
    HELP_KEYBOARD       = 2,
    HELP_HOW_TO_USE_FT2 = 3,
    HELP_FAQ            = 4,
    HELP_KNOWN_BUGS     = 5
};

static char subjectNameBuffer[64], formattedSubjectBuffer[80];
static uint8_t *helpTextBuffer, currTextPalette, currTextFont, helpTextRendered;
static int16_t currTextXPos, currTextYPos;
static uint16_t currSubjectLines;
static int32_t helpYOffset, currSubjectHeight;

static uint16_t controlCodeToNum(char *controlCode)
{
    return ((((controlCode[0] - '0') % 10) * 100) + (((controlCode[1] - '0') % 10) * 10) + ((controlCode[2] - '0') % 10));
}

static int8_t setSubjectAttributes(char *subjectName)
{
    char *lineStartPtr, *linePtr;
    uint8_t lineLen;
    uint16_t controlCode;

    currSubjectHeight = 0;
    currSubjectLines  = 1;

    /* seek to subject text in help data */
    sprintf(formattedSubjectBuffer, "@L%s", subjectName);
    lineStartPtr = strstr((char *)(helpData), formattedSubjectBuffer);
    if (lineStartPtr == NULL)
        return (false);

    lineStartPtr -= 1; /* we must not exclude the very first byte (length) */

    while (!((lineStartPtr[1] == 'E') && (lineStartPtr[2] == 'N') && (lineStartPtr[3] == 'D')))
    {
        currTextFont = FONT_TYPE1;

        lineLen = *((uint8_t *)(lineStartPtr++));
        if (lineLen == 0xFF) /* linefeed only */
        {
            currSubjectLines++;

            if (currTextFont == FONT_TYPE2)
                currSubjectHeight += (FONT2_CHAR_H + 2);
            else
                currSubjectHeight += (FONT1_CHAR_H + 1);

            continue;
        }

        linePtr = lineStartPtr;
        if (*linePtr != ';') /* comment */
        {
            if (*linePtr == '>') /* dummy char */
                 linePtr++;

            controlCode = *((uint16_t *)(linePtr));
            if (controlCode == 0x4C40) /* large font */
                currTextFont = FONT_TYPE2;

            currSubjectLines++;

            if (currTextFont == FONT_TYPE2)
                currSubjectHeight += (FONT2_CHAR_H + 2);
            else
                currSubjectHeight += (FONT1_CHAR_H + 1);
        }

        lineStartPtr += lineLen;
    }

    /* we're always drawing one whole window, so make sure it's not smaller than that */
    if (currSubjectHeight < HELP_WINDOW_HEIGHT)
        currSubjectHeight = HELP_WINDOW_HEIGHT;

    return (true);
}

static void helpCharOut(uint8_t *buffer, uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr, uint8_t fontType)
{
    uint8_t *dstPtr, c, x, y;
    const uint8_t *srcPtr;

    MY_ASSERT(xPos < HELP_TEXT_BUFFER_W)

    c = (uint8_t)(chr);
    if ((c == ' ') || (c >= FONT_CHARS))
        return;

    dstPtr = &buffer[(yPos * HELP_TEXT_BUFFER_W) + xPos];

    if (fontType == FONT_TYPE1) /* normal font */
    {
        srcPtr = &font1Data[c * FONT1_CHAR_W];
        for (y = 0; y < FONT1_CHAR_H; ++y)
        {
            for (x = 0; x < FONT1_CHAR_W; ++x)
            {
                if (srcPtr[x])
                    dstPtr[x] = paletteIndex;
            }

            srcPtr += FONT1_WIDTH;
            dstPtr += HELP_TEXT_BUFFER_W;
        }
    }
    else if (fontType == FONT_TYPE2) /* big font */
    {
        srcPtr = &font2Data[c * FONT2_CHAR_W];
        for (y = 0; y < FONT2_CHAR_H; ++y)
        {
            for (x = 0; x < FONT2_CHAR_W; ++x)
            {
                if (srcPtr[x])
                    dstPtr[x] = paletteIndex;
            }

            srcPtr += FONT2_WIDTH;
            dstPtr += HELP_TEXT_BUFFER_W;
        }
    }
}

static int8_t renderHelpSubjectToBuffer(char *subjectName)
{
    char *lineStartPtr, *linePtr;
    uint8_t lineLen, realLineLen;
    uint16_t controlCode, i, x;

    /* seek to subject text in help data */
    sprintf(formattedSubjectBuffer, "@L%s", subjectName);
    lineStartPtr = strstr((char *)(helpData), formattedSubjectBuffer);
    if (lineStartPtr == NULL)
        return (false); /* help section not found! */

    lineStartPtr -= 1; /* we must not exclude the very first byte (length) */

    currTextXPos    = 0;
    currTextYPos    = 0;
    realLineLen     = 0;
    currTextPalette = PAL_FORGRND;

    while (!((lineStartPtr[1] == 'E') && (lineStartPtr[2] == 'N') && (lineStartPtr[3] == 'D')))
    {
        currTextFont = FONT_TYPE1;

        lineLen = *((uint8_t *)(lineStartPtr++));
        if (lineLen == 0xFF) /* linefeed only */
        {
            lineLen = 0;

            if (currTextFont == FONT_TYPE2)
                currTextYPos += (FONT2_CHAR_H + 2);
            else
                currTextYPos += (FONT1_CHAR_H + 1);

            continue;
        }

        /* ';' on the beginning of a line = comment, don't parse */
        if (*lineStartPtr == ';')
        {
            lineStartPtr += lineLen;
            continue;
        }

        /* do text parsing */

        linePtr = lineStartPtr;
        realLineLen = lineLen;

        /* skip '>' character in beginning of line */
        if (*linePtr == '>')
        {
            linePtr++;
            realLineLen--;
        }

        /* only test control codes if we have room for testing 2 bytes */
        if (realLineLen >= 2)
        {
            controlCode = *((uint16_t *)(linePtr));

            /* large font */
            if (controlCode == 0x4C40)
            {
                linePtr += 2;
                currTextFont    = FONT_TYPE2;
                currTextPalette = PAL_FORGRND;
                realLineLen -= 2;

                if (realLineLen >= 2)
                    controlCode = *((uint16_t *)(linePtr));
            }

            /* column position */
            if (controlCode == 0x5840)
            {
                currTextXPos = 1 + controlCodeToNum(linePtr + 2);
                if (currTextXPos < 0)
                    currTextXPos = 0;

                linePtr     += 5;
                realLineLen -= 5;

                if (realLineLen >= 2)
                    controlCode = *((uint16_t *)(linePtr));
            }

            /* palette */
            if (controlCode == 0x4340)
            {
                currTextPalette = (uint8_t)(controlCodeToNum(linePtr + 2));
                switch (currTextPalette)
                {
                    default:
                    case 1: currTextPalette = PAL_FORGRND; break;
                    case 2: currTextPalette = PAL_BUTTONS; break;
                }

                linePtr     += 5;
                realLineLen -= 5;

                if (realLineLen >= 2)
                    controlCode = *((uint16_t *)(linePtr));
            }

            /* x position */
            if (controlCode == 0x5440)
            {
                currTextXPos = (1 + controlCodeToNum(linePtr + 2)) - 135;
                if (currTextXPos < 0)
                    currTextXPos = 0;

                linePtr     += 5;
                realLineLen -= 5;

                if (realLineLen >= 2)
                    controlCode = *((uint16_t *)(linePtr));
            }
        }

        if (realLineLen > 0)
        {
            x = currTextXPos;
            for (i = 0; i < realLineLen; ++i)
            {
                /* only test control codes if we have room for testing 2 bytes */
                if (i < (realLineLen - 1))
                {
                    controlCode = *((uint16_t *)(&linePtr[i]));
                    if (controlCode == 0x5440) /* x position */
                    {
                        x = 1 + controlCodeToNum(&linePtr[i + 2]);

                        linePtr     += 5;
                        realLineLen -= 5;
                    }
                    else if (controlCode == 0x4340) /* palette */
                    {
                        currTextPalette = (uint8_t)(controlCodeToNum(&linePtr[i + 2]));
                        switch (currTextPalette)
                        {
                            default:
                            case 1: currTextPalette = PAL_FORGRND; break;
                            case 2: currTextPalette = PAL_BUTTONS; break;
                        }

                        linePtr     += 5;
                        realLineLen -= 5;
                    }
                }

                helpCharOut(helpTextBuffer, x, currTextYPos, currTextPalette, linePtr[i], currTextFont);

                if (currTextFont == FONT_TYPE1)
                    x += charWidth(linePtr[i]);
                else
                    x += bigCharWidth(linePtr[i]);
            }
        }

        if (currTextFont == FONT_TYPE1)
            currTextYPos += (FONT1_CHAR_H + 1);
        else
            currTextYPos += (FONT2_CHAR_H + 2);

        lineStartPtr += lineLen;
    }

    return (true);
}

static void updateHelpScreen(void)
{
    MY_ASSERT(currSubjectHeight >= HELP_WINDOW_HEIGHT)
    blitFast(135, 5, &helpTextBuffer[helpYOffset * HELP_TEXT_BUFFER_W], HELP_TEXT_BUFFER_W, HELP_WINDOW_HEIGHT);
}

void freeHelpTextBuffer(void)
{
    if (helpTextBuffer != NULL)
    {
        free(helpTextBuffer);
        helpTextBuffer = NULL;
    }
}

static void renderHelpText(void)
{
    helpTextRendered = false;

    /* set help section compare string (to find the right section of help file) */
    switch (editor.currHelpScreen)
    {
        case HELP_FEATURES:       strcpy(subjectNameBuffer, "Features");                   break;
        case HELP_EFFECTS:        strcpy(subjectNameBuffer, "Effects");                    break;
        case HELP_KEYBOARD:       strcpy(subjectNameBuffer, "Keyboard");                   break;
        case HELP_HOW_TO_USE_FT2: strcpy(subjectNameBuffer, "How to use Fasttracker 2.0"); break;
        case HELP_FAQ:            strcpy(subjectNameBuffer, "Problems/FAQ");               break;
        case HELP_KNOWN_BUGS:     strcpy(subjectNameBuffer, "Known bugs");                 break;
        default: return;
    }

    if (!setSubjectAttributes(subjectNameBuffer))
    {
        clearRect(135, 5, HELP_TEXT_BUFFER_W, HELP_WINDOW_HEIGHT);
        okBox(0, "System message", "Error: Help section not found in help text data!");
        return;
    }

    freeHelpTextBuffer();

    helpTextBuffer = (uint8_t *)(calloc(currSubjectHeight * HELP_TEXT_BUFFER_W, sizeof (int32_t)));
    if (helpTextBuffer == NULL)
    {
        clearRect(134, 5, HELP_TEXT_BUFFER_W, HELP_WINDOW_HEIGHT);
        okBox(0, "System message", "Not enough memory!");
        return;
    }

    if (!renderHelpSubjectToBuffer(subjectNameBuffer))
    {
        clearRect(134, 5, HELP_TEXT_BUFFER_W, HELP_WINDOW_HEIGHT);
        okBox(0, "System message", "Error: Couldn't render help text! Parsing error?");
        return;
    }

    helpTextRendered = true;

    setScrollBarEnd(SB_HELP_SCROLL, currSubjectLines);
    updateHelpScreen();
}

void helpScrollUp(void)
{
    if (helpTextRendered && (currSubjectLines > HELP_WINDOW_LINES))
    {
        if (helpYOffset >= HELP_LINE_HEIGHT)
        {
            helpYOffset -= HELP_LINE_HEIGHT;
            scrollBarScrollUp(SB_HELP_SCROLL, 1);
            updateHelpScreen();
        }
    }
}

void helpScrollDown(void)
{
    if (helpTextRendered && (currSubjectLines > HELP_WINDOW_LINES))
    {
        if (helpYOffset <= (currSubjectHeight - HELP_LINE_HEIGHT - HELP_WINDOW_HEIGHT))
        {
            helpYOffset += HELP_LINE_HEIGHT;
            scrollBarScrollDown(SB_HELP_SCROLL, 1);
            updateHelpScreen();
        }
    }
}

void helpScrollSetPos(int32_t pos)
{
    if (!helpTextRendered)
        return;

    pos *= HELP_LINE_HEIGHT;
    if (helpYOffset != pos)
    {
        helpYOffset = pos;
        updateHelpScreen();
    }
}

void showHelpScreen(void)
{
    uint16_t tmpID;

    if (editor.ui.extended)
        exitPatternEditorExtended();

    hideTopScreen();
    editor.ui.helpScreenShown = true;

    drawFramework(0,   0, 128, 173, FRAMEWORK_TYPE1);
    drawFramework(128, 0, 504, 173, FRAMEWORK_TYPE1);
    drawFramework(130, 2, 479, 169, FRAMEWORK_TYPE2);

    showPushButton(PB_HELP_EXIT);
    showPushButton(PB_HELP_SCROLL_UP);
    showPushButton(PB_HELP_SCROLL_DOWN);

    uncheckRadioButtonGroup(RB_GROUP_HELP);
    switch (editor.currHelpScreen)
    {
        default:
        case HELP_FEATURES:       tmpID = RB_HELP_FEATURES;       break;
        case HELP_EFFECTS:        tmpID = RB_HELP_EFFECTS;        break;
        case HELP_KEYBOARD:       tmpID = RB_HELP_KEYBOARD;       break;
        case HELP_HOW_TO_USE_FT2: tmpID = RB_HELP_HOW_TO_USE_FT2; break;
        case HELP_FAQ:            tmpID = RB_HELP_FAQ;            break;
        case HELP_KNOWN_BUGS:     tmpID = RB_HELP_KNOWN_BUGS;     break;
    }
    radioButtons[tmpID].state = RADIOBUTTON_CHECKED;

    showRadioButtonGroup(RB_GROUP_HELP);

    showScrollBar(SB_HELP_SCROLL);

    textOutShadow(4,   3, PAL_FORGRND, PAL_DSKTOP2, "Subjects:");
    textOutShadow(20, 17, PAL_FORGRND, PAL_DSKTOP2, "Features");
    textOutShadow(20, 32, PAL_FORGRND, PAL_DSKTOP2, "Effects");
    textOutShadow(20, 47, PAL_FORGRND, PAL_DSKTOP2, "Keyboard");
    textOutShadow(20, 62, PAL_FORGRND, PAL_DSKTOP2, "How to use FT2");
    textOutShadow(20, 77, PAL_FORGRND, PAL_DSKTOP2, "Problems/FAQ");
    textOutShadow(20, 92, PAL_FORGRND, PAL_DSKTOP2, "Known bugs");

    renderHelpText();
}

void hideHelpScreen(void)
{
    freeHelpTextBuffer();

    hidePushButton(PB_HELP_EXIT);
    hidePushButton(PB_HELP_SCROLL_UP);
    hidePushButton(PB_HELP_SCROLL_DOWN);
    hideRadioButtonGroup(RB_GROUP_HELP);
    hideScrollBar(SB_HELP_SCROLL);

    editor.ui.helpScreenShown = false;
}

void exitHelpScreen(void)
{
    hideHelpScreen();
    showTopScreen(true);
}

void rbHelpFeatures(void)
{
    checkRadioButton(RB_HELP_FEATURES);
    editor.currHelpScreen = HELP_FEATURES;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}

void rbHelpEffects(void)
{
    checkRadioButton(RB_HELP_EFFECTS);
    editor.currHelpScreen = HELP_EFFECTS;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}

void rbHelpKeyboard(void)
{
    checkRadioButton(RB_HELP_KEYBOARD);
    editor.currHelpScreen = HELP_KEYBOARD;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}

void rbHelpHowToUseFT2(void)
{
    checkRadioButton(RB_HELP_HOW_TO_USE_FT2);
    editor.currHelpScreen = HELP_HOW_TO_USE_FT2;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}

void rbHelpFAQ(void)
{
    checkRadioButton(RB_HELP_FAQ);
    editor.currHelpScreen = HELP_FAQ;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}

void rbHelpKnownBugs(void)
{
    checkRadioButton(RB_HELP_KNOWN_BUGS);
    editor.currHelpScreen = HELP_KNOWN_BUGS;
    setScrollBarPos(SB_HELP_SCROLL, 0, true);
    renderHelpText();
}
