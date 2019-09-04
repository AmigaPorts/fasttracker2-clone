/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdint.h>
#include <time.h>
#include "ft2_header.h"
#include "ft2_gfxdata.h"
#include "ft2_config.h"
#include "ft2_about.h"
#include "ft2_mouse.h"
#include "ft2_nibbles.h"
#include "ft2_gui.h"
#include "ft2_pattern_ed.h"
#include "ft2_scopes.h"
#include "ft2_help.h"
#include "ft2_sample_ed.h"
#include "ft2_inst_ed.h"
#include "ft2_diskop.h"
#include "ft2_wav_renderer.h"
#include "ft2_trim.h"
#include "ft2_video.h"

int8_t setupGUI(void)
{
    int16_t i;
    textBox_t *t;

    /* all memory will be NULL-tested and free'd if we return false somewhere in this function */

    editor.blkCopyBuff = (tonTyp *)(calloc(MAX_PATT_LEN * MAX_VOICES, sizeof (tonTyp)));
    if (editor.blkCopyBuff == NULL)
        return (false);

    editor.ptnCopyBuff = (tonTyp *)(calloc(MAX_PATT_LEN * MAX_VOICES, sizeof (tonTyp)));
    if (editor.ptnCopyBuff == NULL)
        return (false);

    editor.trackCopyBuff = (tonTyp *)(calloc(MAX_PATT_LEN, sizeof (tonTyp)));
    if (editor.trackCopyBuff == NULL)
        return (false);

    editor.tmpFilenameU = (UNICHAR *)(calloc(PATH_MAX + 1, sizeof (UNICHAR)));
    if (editor.tmpFilenameU == NULL)
        return (false);

    editor.tmpInstrFilenameU = (UNICHAR *)(calloc(PATH_MAX + 1, sizeof (UNICHAR)));
    if (editor.tmpInstrFilenameU == NULL)
        return (false);

    /* setup text edit boxes */
    for (i = 0; i < NUM_TEXTBOXES; ++i)
    {
        t = &textBoxes[i];

        t->visible         = false;
        t->bufOffset       = 0;
        t->cursorPos       = 0;
        t->textPtr         = NULL;
        t->renderBufWidth  = (9 + 1) * t->maxChars; /* 9 = max character/glyph width possible */
        t->renderBufHeight = 10; /* 10 = max character height possible */

        t->renderBuf = (uint8_t *)(calloc(t->renderBufWidth * t->renderBufHeight, sizeof (int8_t)));
        if (t->renderBuf == NULL)
            return (false);
    }

    seedAboutScreenRandom((uint32_t)(time(NULL)));
    setupInitialTextBoxPointers();
    setInitialTrimFlags();
    initializeScrollBars();
    setMouseMode(MOUSE_MODE_NORMAL);
    updateTextBoxPointers();
    drawGUIOnRunTime();
    updateSampleEditorSample();
    updatePatternWidth();
    updateMouseScaling();

    return (true);
}

/* TEXT ROUTINES */

char relocateChars(char ch, int8_t fontType)
{
    switch (fontType)
    {
        /* standard GUI font */
        case FONT_TYPE1:
        {
            if (ch >= 0)
                return (ch);

            /* codepage 437 nordic characters */
                 if (ch == -124) ch = 4;
            else if (ch == -108) ch = 20;
            else if (ch == -122) ch = 6;
            else if (ch == -114) ch = 14;
            else if (ch == -103) ch = 25;
            else if (ch == -113) ch = 15;
        }
        break;

        case FONT_TYPE2:
        {
            if (ch >= 0)
                return (ch);

            /* codepage 437 nordic characters */
                 if (ch == -124) ch = 24;
            else if (ch == -108) ch = 25;
            else if (ch == -122) ch = 26;
            else if (ch == -114) ch = 21;
            else if (ch == -103) ch = 22;
            else if (ch == -113) ch = 23;
        }
        break;

        case FONT_TYPE3: /* small pattern font */
        {
            /* characters */
            if ((ch >= '0') && (ch <= '9'))
                ch -= '0';
            else if ((ch >= 'A') && (ch <= 'Z'))
                ch -= ('A' - 10);
            else
                ch = ' ';
        }
        break;

        case FONT_TYPE4: /* medium pattern font */
        {
            /* characters */
            if ((ch >= '0') && (ch <= '9'))
                ch -= '0';
            else if (ch >= 'A')
                ch -= ('A' - 10);
            else
                ch = ' ';
        }
        break;

        case FONT_TYPE5: /* big pattern font */
        {
            if ((ch >= '0') && (ch <= '9'))
                ch -= '0';
            else if (ch >= 'A')
                ch -= ('A' - 10);
            else
                ch = ' ';
        }
        break;

        case FONT_TYPE6: /* hex font */
        {
            if ((ch >= '0') && (ch <= '9'))
                ch -= '0';
            else if ((ch >= 'A') && (ch <= 'F'))
                ch -= ('A' - 10);
        }
        break;

        case FONT_TYPE7: /* tiny pattern note font */
        {
                 if ((ch >= '0') && (ch <= '7')) ch -= '0';
            else if ((ch >= 'C') && (ch <= 'G')) ch = 8 + (ch - 'C');
            else if (ch == 'A') ch = 13;
            else ch = ' ';
        }
        break;

        default:
            break;
    }

    /* really important! font has 128 chars not 256 */
    if (ch < 0)
        ch = ' ';

    return (ch);
}

int8_t getCharWidth(char ch, int8_t fontType)
{
    const uint8_t *widthPtr;

    switch (fontType)
    {
        case FONT_TYPE1: widthPtr = font1Widths; break;
        case FONT_TYPE2: widthPtr = font2Widths; break;
        case FONT_TYPE3: return (4);
        case FONT_TYPE4: return (8);
        case FONT_TYPE5: return (16);
        case FONT_TYPE6: return (8);
        case FONT_TYPE7: return (6);
        default:         return (0);
    }

   ch = relocateChars(ch, fontType);
   return (widthPtr[(uint32_t)(ch)]);
}

/* normal font, no relocate for extended characters */
void charOutFast(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr)
{
    const uint8_t *srcPtr;
    uint32_t x, y, *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    pixVal = video.palette[paletteIndex];
    srcPtr = &font1Data[chr * FONT1_CHAR_W];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];

    for (y = 0; y < FONT1_CHAR_H; ++y)
    {
        for (x = 0; x < FONT1_CHAR_W; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
        }

        srcPtr += FONT1_WIDTH;
        dstPtr += SCREEN_W;
    }
}

void charOutFastOutlined(uint16_t x, uint16_t y, uint8_t paletteIndex, char chr)
{
    charOutFast(x - 1, y, PAL_BCKGRND, chr);
    charOutFast(x + 1, y, PAL_BCKGRND, chr);
    charOutFast(x, y - 1, PAL_BCKGRND, chr);
    charOutFast(x, y + 1, PAL_BCKGRND, chr);

    charOutFast(x, y, paletteIndex, chr);
}

/* normal font */
void charOut(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr)
{
    const uint8_t *srcPtr;
    uint8_t x, y;
    uint32_t *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    if (chr == ' ')
        return;

    srcPtr = &font1Data[relocateChars(chr, FONT_TYPE1) * FONT1_CHAR_W];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    pixVal = video.palette[paletteIndex];

    for (y = 0; y < FONT1_CHAR_H; ++y)
    {
        for (x = 0; x < FONT1_CHAR_W; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
        }

        srcPtr += FONT1_WIDTH;
        dstPtr += SCREEN_W;
    }
}

/* big font */
void charBigOut(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr)
{
    const uint8_t *srcPtr;
    uint8_t x, y;
    uint32_t *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    if (chr == ' ')
        return;

    srcPtr = &font2Data[relocateChars(chr, FONT_TYPE2) * FONT2_CHAR_W];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    pixVal = video.palette[paletteIndex];

    for (y = 0; y < FONT2_CHAR_H; ++y)
    {
        for (x = 0; x < FONT2_CHAR_W; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
        }

        srcPtr += FONT2_WIDTH;
        dstPtr += SCREEN_W;
    }
}

void charBigOutFast(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr)
{
    const uint8_t *srcPtr;
    uint8_t x, y;
    uint32_t *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    if (chr == ' ')
        return;

    pixVal = video.palette[paletteIndex];
    srcPtr = &font2Data[chr * FONT2_CHAR_W];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];

    for (y = 0; y < FONT2_CHAR_H; ++y)
    {
        for (x = 0; x < FONT2_CHAR_W; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
        }

        srcPtr += FONT2_WIDTH;
        dstPtr += SCREEN_W;
    }
}

void charOutClipped(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, char chr, uint16_t clipX)
{
    const uint8_t *srcPtr;
    uint8_t x, y;
    uint32_t *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    if (chr == ' ')
        return;

    pixVal = video.palette[paletteIndex];
    srcPtr = &font1Data[chr * FONT1_CHAR_W];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];

    for (y = 0; y < FONT1_CHAR_H; ++y)
    {
        for (x = 0; x < FONT1_CHAR_W; ++x)
        {
            if ((x + xPos) >= clipX)
            {
                srcPtr++;
                dstPtr++;
                continue;
            }

            if (*srcPtr++)
                *dstPtr = pixVal;

            dstPtr++;
        }

        srcPtr += (FONT1_WIDTH - FONT1_CHAR_W);
        dstPtr += (SCREEN_W    - FONT1_CHAR_W);
    }
}

void charOutShadow(uint16_t x, uint16_t y, uint8_t paletteIndex, uint8_t shadowPaletteIndex, char chr)
{
    /* clipping is done in charOut() */
    charOut(x + 1, y + 1, shadowPaletteIndex, chr); /* shadow */
    charOut(x + 0, y + 0, paletteIndex,       chr); /* foreground */
}

void textOut(uint16_t x, uint16_t y, uint8_t paletteIndex, char *textPtr)
{
    char ch;
    uint16_t currX;

    MY_ASSERT(textPtr != NULL)

    currX = x;
    while (*textPtr != '\0')
    {
        ch = relocateChars(*textPtr++, FONT_TYPE1);

        charOutFast(currX, y, paletteIndex, ch);
        currX += font1Widths[(uint32_t)(ch)];
    }
}

void textBigOut(uint16_t x, uint16_t y, uint8_t paletteIndex, char *textPtr)
{
    char ch;
    uint16_t currX;

    MY_ASSERT(textPtr != NULL)

    currX = x;
    while (*textPtr != '\0')
    {
        ch = relocateChars(*textPtr++, FONT_TYPE2);

        charBigOutFast(currX, y, paletteIndex, ch);
        currX += font2Widths[(uint32_t)(ch)];
    }
}

void textBigOutShadow(uint16_t x, uint16_t y, uint8_t paletteIndex, uint8_t shadowPaletteIndex, char *textPtr)
{
    /* clipping is done in charOut() */
    textBigOut(x + 1, y + 1, shadowPaletteIndex, textPtr); /* shadow */
    textBigOut(x + 0, y + 0, paletteIndex,       textPtr); /* foreground */
}

void textOutClipped(uint16_t x, uint16_t y, uint8_t paletteIndex, char *textPtr, uint16_t clipX)
{
    char ch;
    uint16_t currX;

    MY_ASSERT(textPtr != NULL)

    currX = x;
    while (*textPtr != '\0')
    {
        ch = relocateChars(*textPtr++, FONT_TYPE1);

        charOutClipped(currX, y, paletteIndex, ch, clipX);

        currX += font1Widths[(uint32_t)(ch)];
        if (currX >= clipX)
            break;
    }
}

void textOutShadowClipped(uint16_t x, uint16_t y, uint8_t paletteIndex, uint8_t shadowPaletteIndex, char *textPtr, uint16_t clipX)
{
    /* clipping is done in charOutClipped() */
    textOutClipped(x + 1, y + 1, shadowPaletteIndex, textPtr, clipX); /* shadow */
    textOutClipped(x + 0, y + 0, paletteIndex,       textPtr, clipX); /* foreground */
}

void textOutShadow(uint16_t x, uint16_t y, uint8_t paletteIndex, uint8_t shadowPaletteIndex, char *textPtr)
{
    textOut(x + 1, y + 1, shadowPaletteIndex, textPtr); /* shadow */
    textOut(x + 0, y + 0, paletteIndex,       textPtr); /* foreground */
}

void drawSmallHex(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, uint8_t val)
{
    const uint8_t *srcPtr;
    uint32_t x, y, *dstPtr, pixVal;

    MY_ASSERT(val <= 0xF)

    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    srcPtr = &smallHexBitmap[val * 5];

    pixVal = video.palette[paletteIndex];
    for (y = 0; y < 7; ++y)
    {
        for (x = 0; x < 5; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
        }

        dstPtr += SCREEN_W;
        srcPtr += 80;
    }
}

void drawSmallHexBg(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, uint8_t bgPaletteIndex, uint8_t val)
{
    const uint8_t *srcPtr;
    uint32_t x, y, *dstPtr, pixVal, bgPixVal;

    MY_ASSERT(val <= 0xF)

    dstPtr   = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    srcPtr   = &smallHexBitmap[val * 5];
    pixVal   = video.palette[paletteIndex];
    bgPixVal = video.palette[bgPaletteIndex];

    for (y = 0; y < 7; ++y)
    {
        for (x = 0; x < 5; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = pixVal;
            else
                dstPtr[x] = bgPixVal;
        }

        dstPtr += SCREEN_W;
        srcPtr += 80;
    }
}

void hexOut(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, uint32_t val, uint8_t numDigits)
{
    const uint8_t *srcPtr;
    int8_t i;
    uint8_t x, y, nybble;
    uint32_t *dstPtr, pixVal;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H))

    pixVal = video.palette[paletteIndex];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];

    for (i = (numDigits - 1); i >= 0; --i)
    {
        /* extract current nybble and set pointer to glyph */
        nybble = (val >> (i * 4)) & 15;
        srcPtr = &font6Data[nybble * FONT6_CHAR_W];

        /* render glyph */
        for (y = 0; y < FONT6_CHAR_H; ++y)
        {
            for (x = 0; x < FONT6_CHAR_W; ++x)
            {
                if (srcPtr[x])
                    dstPtr[x] = pixVal;
            }

            srcPtr += FONT6_WIDTH;
            dstPtr += SCREEN_W;
        }

        /* xpos += FONT6_CHAR_W */
        dstPtr -= ((SCREEN_W * FONT6_CHAR_H) - FONT6_CHAR_W);
    }
}

void hexOutShadow(uint16_t xPos, uint16_t yPos, uint8_t paletteIndex, uint8_t shadowPaletteIndex, uint32_t val, uint8_t numDigits)
{
    hexOut(xPos + 1, yPos + 1, shadowPaletteIndex, val, numDigits);
    hexOut(xPos + 0, yPos + 0,       paletteIndex, val, numDigits);
}

/* return full pixel width of a text string */
uint16_t getTextWidth(char *textPtr, uint8_t fontType)
{
    uint16_t textWidth;

    MY_ASSERT(textPtr != NULL)

    textWidth = 0;
    while (*textPtr != '\0')
        textWidth += getCharWidth(*textPtr++, fontType);

    /* there will be a pixel spacer at the end of the last char, remove it */
    if (textWidth > 0)
        textWidth--;

    return (textWidth);
}

/* FILL ROUTINES */

void clearRect(uint16_t xPos, uint16_t yPos, uint16_t w, uint16_t h)
{
    uint16_t y;
    uint32_t *dstPtr, fillNumDwords;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H) && ((xPos + w) <= SCREEN_W) && ((yPos + h) <= SCREEN_H))

    fillNumDwords = w * sizeof (uint32_t);

    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    for (y = 0; y < h; ++y)
    {
        memset(dstPtr, 0, fillNumDwords);
        dstPtr += SCREEN_W;
    }
}

void fillRect(uint16_t xPos, uint16_t yPos, uint16_t w, uint16_t h, uint8_t paletteIndex)
{
    uint32_t *dstPtr, pixVal, x, y;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H) && ((xPos + w) <= SCREEN_W) && ((yPos + h) <= SCREEN_H))

    pixVal = video.palette[paletteIndex];
    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];

    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
            dstPtr[x] = pixVal;

        dstPtr += SCREEN_W;
    }
}

void blit(uint16_t xPos, uint16_t yPos, const uint8_t *srcPtr, uint16_t w, uint16_t h)
{
    uint32_t *dstPtr, x, y, pixel;

    MY_ASSERT((srcPtr != NULL) && (xPos < SCREEN_W) && (yPos < SCREEN_H) && ((xPos + w) <= SCREEN_W) && ((yPos + h) <= SCREEN_H))

    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
        {
            pixel = srcPtr[x];
            if (pixel != PAL_TRANSPR)
                dstPtr[x] = video.palette[pixel];
        }

        srcPtr += w;
        dstPtr += SCREEN_W;
    }
}

void blitFast(uint16_t xPos, uint16_t yPos, const uint8_t *srcPtr, uint16_t w, uint16_t h) /* no colorkey */
{
    uint32_t *dstPtr, x, y;

    MY_ASSERT((srcPtr != NULL) && (xPos < SCREEN_W) && (yPos < SCREEN_H) && ((xPos + w) <= SCREEN_W) && ((yPos + h) <= SCREEN_H))

    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < w; ++x)
            dstPtr[x] = video.palette[srcPtr[x]];

        srcPtr += w;
        dstPtr += SCREEN_W;
    }
}

/* LINE ROUTINES */

void hLine(uint16_t x, uint16_t y, uint16_t w, uint8_t paletteIndex)
{
    uint32_t *dstPtr, i, pixVal;

    MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H) && ((x + w) <= SCREEN_W))

    pixVal = video.palette[paletteIndex];

    dstPtr = &video.frameBuffer[(y * SCREEN_W) + x];
    for (i = 0; i < w; ++i)
        dstPtr[i] = pixVal;
}

void vLine(uint16_t x, uint16_t y, uint16_t h, uint8_t paletteIndex)
{
    uint32_t *dstPtr,i, pixVal;

    MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H) && ((y + h) <= SCREEN_W))

    pixVal = video.palette[paletteIndex];

    dstPtr = &video.frameBuffer[(y * SCREEN_W) + x];
    for (i = 0; i < h; ++i)
    {
        *dstPtr  = pixVal;
         dstPtr += SCREEN_W;
    }
}

void line(int16_t x1, int16_t x2, int16_t y1, int16_t y2, uint8_t paletteIndex)
{
    int16_t d, x, y, sx, sy, dx, dy;
    uint16_t ax, ay;
    int32_t pitch;
    uint32_t pixVal, *dst32;

    /* get coefficients */
    dx = x2 - x1;
    ax = ABS(dx) * 2;
    sx = SGN(dx);
    dy = y2 - y1;
    ay = ABS(dy) * 2;
    sy = SGN(dy);
    x  = x1;
    y  = y1;

    pixVal = video.palette[paletteIndex];
    pitch  = sy * SCREEN_W;
    dst32  = &video.frameBuffer[(y * SCREEN_W) + x];

    /* draw line */
    if (ax > ay)
    {
        d = ay - (ax / 2);

        while (true)
        {
            MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H))

            *dst32 = pixVal;
            if (x == x2)
                break;

            if (d >= 0)
            {
#ifdef _DEBUG
                y += sy;
#endif
                d -= ax;
                dst32 += pitch;
            }

            x += sx;
            d += ay;
            dst32 += sx;
        }
    }
    else
    {
        d = ax - (ay / 2);

        while (true)
        {
            MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H))

            *dst32 = pixVal;
            if (y == y2)
                break;

            if (d >= 0)
            {
#ifdef _DEBUG
                x += sx;
#endif
                d -= ay;
                dst32 += sx;
            }

            y += sy;
            d += ax;
            dst32 += pitch;
        }
    }
}

void drawFramework(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t type)
{
    MY_ASSERT((x < SCREEN_W) && (y < SCREEN_H) && (w >= 2) && (h >= h))

    h--;
    w--;

    if (type == FRAMEWORK_TYPE1)
    {
        /* top left corner */
        hLine(x, y,     w,     PAL_DSKTOP1);
        vLine(x, y + 1, h - 1, PAL_DSKTOP1);

        /* bottom right corner */
        hLine(x,     y + h, w,     PAL_DSKTOP2);
        vLine(x + w, y,     h + 1, PAL_DSKTOP2);

        /* fill background */
        fillRect(x + 1, y + 1, w - 1, h - 1, PAL_DESKTOP);
    }
    else
    {
        /* top left corner */
        hLine(x, y,     w + 1, PAL_DSKTOP2);
        vLine(x, y + 1, h,     PAL_DSKTOP2);

        /* bottom right corner */
        hLine(x + 1, y + h, w,     PAL_DSKTOP1);
        vLine(x + w, y + 1, h - 1, PAL_DSKTOP1);

        /* clear background */
        clearRect(x + 1, y + 1, w - 1, h - 1);
    }
}

/* GUI FUNCTIONS */

void showTopLeftMainScreen(uint8_t restoreScreens)
{
    editor.ui.diskOpShown          = false;
    editor.ui.sampleEditorExtShown = false;
    editor.ui.instEditorExtShown   = false;
    editor.ui.transposeShown       = false;
    editor.ui.advEditShown         = false;
    editor.ui.wavRendererShown     = false;
    editor.ui.trimScreenShown      = false;

    editor.ui.scopesShown = true;
    if (restoreScreens)
    {
        switch (editor.ui.oldTopLeftScreen)
        {
            default: break;
            case 1: editor.ui.diskOpShown          = true; break;
            case 2: editor.ui.sampleEditorExtShown = true; break;
            case 3: editor.ui.instEditorExtShown   = true; break;
            case 4: editor.ui.transposeShown       = true; break;
            case 5: editor.ui.advEditShown         = true; break;
            case 6: editor.ui.wavRendererShown     = true; break;
            case 7: editor.ui.trimScreenShown      = true; break;
        }

        if (editor.ui.oldTopLeftScreen > 0)
            editor.ui.scopesShown = false;
    }

    editor.ui.oldTopLeftScreen = 0;

    if (editor.ui.diskOpShown)
    {
        showDiskOpScreen();
    }
    else
    {
        /* pos ed. */
        drawFramework(0, 0, 112, 77, FRAMEWORK_TYPE1);
        drawFramework(2, 2,  51, 19, FRAMEWORK_TYPE2);
        drawFramework(2,30,  51, 19, FRAMEWORK_TYPE2);
        showScrollBar(SB_POS_ED);
        showPushButton(PB_POSED_POS_UP);
        showPushButton(PB_POSED_POS_DOWN);
        showPushButton(PB_POSED_INS);
        showPushButton(PB_POSED_PATT_UP);
        showPushButton(PB_POSED_PATT_DOWN);
        showPushButton(PB_POSED_DEL);
        showPushButton(PB_POSED_LEN_UP);
        showPushButton(PB_POSED_LEN_DOWN);
        showPushButton(PB_POSED_REP_UP);
        showPushButton(PB_POSED_REP_DOWN);
        textOutShadow(4, 52, PAL_FORGRND, PAL_DSKTOP2, "Songlen.");
        textOutShadow(4, 64, PAL_FORGRND, PAL_DSKTOP2, "Repstart");
        drawPosEdNums(song.songPos);
        drawSongLength();
        drawSongRepS();

        /* logo button */
        showPushButton(PB_LOGO);
        showPushButton(PB_BADGE);

        /* left menu */
        drawFramework(291, 0, 65, 173, FRAMEWORK_TYPE1);
        showPushButton(PB_ABOUT);
        showPushButton(PB_NIBBLES);
        showPushButton(PB_KILL);
        showPushButton(PB_TRIM);
        showPushButton(PB_EXTEND_VIEW);
        showPushButton(PB_TRANSPOSE);
        showPushButton(PB_INST_ED_EXT);
        showPushButton(PB_SMP_ED_EXT);
        showPushButton(PB_ADV_EDIT);
        showPushButton(PB_ADD_CHANNELS);
        showPushButton(PB_SUB_CHANNELS);

        /* song/pattern */
        drawFramework(112, 32, 94, 45, FRAMEWORK_TYPE1);
        drawFramework(206, 32, 85, 45, FRAMEWORK_TYPE1);
        showPushButton(PB_BPM_UP);
        showPushButton(PB_BPM_DOWN);
        showPushButton(PB_SPEED_UP);
        showPushButton(PB_SPEED_DOWN);
        showPushButton(PB_EDITADD_UP);
        showPushButton(PB_EDITADD_DOWN);
        showPushButton(PB_PATT_UP);
        showPushButton(PB_PATT_DOWN);
        showPushButton(PB_PATTLEN_UP);
        showPushButton(PB_PATTLEN_DOWN);
        showPushButton(PB_PATT_EXPAND);
        showPushButton(PB_PATT_SHRINK);
        textOutShadow(116, 36, PAL_FORGRND, PAL_DSKTOP2, "BPM");
        textOutShadow(116, 50, PAL_FORGRND, PAL_DSKTOP2, "Spd.");
        textOutShadow(116, 64, PAL_FORGRND, PAL_DSKTOP2, "Add.");
        textOutShadow(210, 36, PAL_FORGRND, PAL_DSKTOP2, "Ptn.");
        textOutShadow(210, 50, PAL_FORGRND, PAL_DSKTOP2, "Ln.");
        drawSongBPM(song.speed);
        drawSongSpeed(song.tempo);
        drawEditPattern(editor.editPattern);
        drawPatternLength(editor.editPattern);
        drawEditSkip();

        /* status bar */
        drawFramework(0, 77, 291, 15, FRAMEWORK_TYPE1);
        textOutShadow(4, 80, PAL_FORGRND, PAL_DSKTOP2, "Global volume");
        drawGlobalVol(song.globVol);

        editor.updatePosSections = true;

        textOutShadow(204, 80, PAL_FORGRND, PAL_DSKTOP2, "Time");
        charOutShadow(250, 80, PAL_FORGRND, PAL_DSKTOP2, ':');
        charOutShadow(270, 80, PAL_FORGRND, PAL_DSKTOP2, ':');
        drawPlaybackTime();

        if (editor.ui.sampleEditorExtShown)
            drawSampleEditorExt();
        else if (editor.ui.instEditorExtShown)
            drawInstEditorExt();
        else if (editor.ui.transposeShown)
            drawTranspose();
        else if (editor.ui.advEditShown)
            drawAdvEdit();
        else if (editor.ui.wavRendererShown)
            drawWavRenderer();
        else if (editor.ui.trimScreenShown)
            drawTrimScreen();

        if (editor.ui.scopesShown)
            drawScopeFramework();
    }
}

void hideTopLeftMainScreen(void)
{
    hideDiskOpScreen();
    hideInstEditorExt();
    hideSampleEditorExt();
    hideTranspose();
    hideAdvEdit();
    hideWavRenderer();
    hideTrimScreen();

    editor.ui.scopesShown = false;

    /* position editor */
    hideScrollBar(SB_POS_ED);

    hidePushButton(PB_POSED_POS_UP);
    hidePushButton(PB_POSED_POS_DOWN);
    hidePushButton(PB_POSED_INS);
    hidePushButton(PB_POSED_PATT_UP);
    hidePushButton(PB_POSED_PATT_DOWN);
    hidePushButton(PB_POSED_DEL);
    hidePushButton(PB_POSED_LEN_UP);
    hidePushButton(PB_POSED_LEN_DOWN);
    hidePushButton(PB_POSED_REP_UP);
    hidePushButton(PB_POSED_REP_DOWN);

    /* logo button */
    hidePushButton(PB_LOGO);
    hidePushButton(PB_BADGE);

    /* left menu */
    hidePushButton(PB_ABOUT);
    hidePushButton(PB_NIBBLES);
    hidePushButton(PB_KILL);
    hidePushButton(PB_TRIM);
    hidePushButton(PB_EXTEND_VIEW);
    hidePushButton(PB_TRANSPOSE);
    hidePushButton(PB_INST_ED_EXT);
    hidePushButton(PB_SMP_ED_EXT);
    hidePushButton(PB_ADV_EDIT);
    hidePushButton(PB_ADD_CHANNELS);
    hidePushButton(PB_SUB_CHANNELS);

    /* song/pattern */
    hidePushButton(PB_BPM_UP);
    hidePushButton(PB_BPM_DOWN);
    hidePushButton(PB_SPEED_UP);
    hidePushButton(PB_SPEED_DOWN);
    hidePushButton(PB_EDITADD_UP);
    hidePushButton(PB_EDITADD_DOWN);
    hidePushButton(PB_PATT_UP);
    hidePushButton(PB_PATT_DOWN);
    hidePushButton(PB_PATTLEN_UP);
    hidePushButton(PB_PATTLEN_DOWN);
    hidePushButton(PB_PATT_EXPAND);
    hidePushButton(PB_PATT_SHRINK);
}

void showTopRightMainScreen(void)
{
    /* right menu */
    drawFramework(356, 0, 65, 173, FRAMEWORK_TYPE1);
    showPushButton(PB_PLAY_SONG);
    showPushButton(PB_PLAY_PATT);
    showPushButton(PB_STOP);
    showPushButton(PB_RECORD_SONG);
    showPushButton(PB_RECORD_PATT);
    showPushButton(PB_DISK_OP);
    showPushButton(PB_INST_ED);
    showPushButton(PB_SMP_ED);
    showPushButton(PB_CONFIG);
    showPushButton(PB_HELP);

    /* instrument switcher */
    editor.ui.instrSwitcherShown = true;
    showInstrumentSwitcher();

    /* song name */
    showTextBox(TB_SONG_NAME);
    drawSongName();
}

void hideTopRightMainScreen(void)
{
    /* right menu */
    hidePushButton(PB_PLAY_SONG);
    hidePushButton(PB_PLAY_PATT);
    hidePushButton(PB_STOP);
    hidePushButton(PB_RECORD_SONG);
    hidePushButton(PB_RECORD_PATT);
    hidePushButton(PB_DISK_OP);
    hidePushButton(PB_INST_ED);
    hidePushButton(PB_SMP_ED);
    hidePushButton(PB_CONFIG);
    hidePushButton(PB_HELP);

    /* instrument switcher */
    hideInstrumentSwitcher();
    editor.ui.instrSwitcherShown = false;

    hideTextBox(TB_SONG_NAME);
}

/* BOTTOM STUFF */

void setOldTopLeftScreenFlag(void)
{
    if (editor.ui.diskOpShown)
        editor.ui.oldTopLeftScreen = 1;
    else if (editor.ui.sampleEditorExtShown)
        editor.ui.oldTopLeftScreen = 2;
    else if (editor.ui.instEditorExtShown)
        editor.ui.oldTopLeftScreen = 3;
    else if (editor.ui.transposeShown)
        editor.ui.oldTopLeftScreen = 4;
    else if (editor.ui.advEditShown)
        editor.ui.oldTopLeftScreen = 5;
    else if (editor.ui.wavRendererShown)
        editor.ui.oldTopLeftScreen = 6;
    else if (editor.ui.trimScreenShown)
        editor.ui.oldTopLeftScreen = 7;
}

void hideTopLeftScreen(void)
{
    setOldTopLeftScreenFlag();

    hideTopLeftMainScreen();
    hideNibblesScreen();
    hideConfigScreen();
    hideAboutScreen();
    hideHelpScreen();
}

void hideTopScreen(void)
{
    setOldTopLeftScreenFlag();

    hideTopLeftMainScreen();
    hideTopRightMainScreen();
    hideNibblesScreen();
    hideConfigScreen();
    hideAboutScreen();
    hideHelpScreen();

    editor.ui.instrSwitcherShown = false;
    editor.ui.scopesShown = false;
}

void showTopScreen(uint8_t restoreScreens)
{
    editor.ui.scopesShown = false;

    if (editor.ui.aboutScreenShown)
    {
        showAboutScreen();
    }
    else if (editor.ui.configScreenShown)
    {
        showConfigScreen();
    }
    else if (editor.ui.helpScreenShown)
    {
        showHelpScreen();
    }
    else if (editor.ui.nibblesShown)
    {
        showNibblesScreen();
    }
    else
    {
        showTopLeftMainScreen(restoreScreens); /* updates editor.ui.scopesShown */
        showTopRightMainScreen();
    }
}

void showBottomScreen(void)
{
    if (editor.ui.extended || editor.ui.patternEditorShown)
    {
        showPatternEditor();
    }
    else if (editor.ui.instEditorShown)
    {
        showInstEditor();
    }
    else if (editor.ui.sampleEditorShown)
    {
        showSampleEditor();
    }
}

void drawGUIOnRunTime(void)
{
    setScrollBarPos(SB_POS_ED, 0, false);

    showTopScreen(false); /* false = don't restore screens */
    showPatternEditor();

    editor.updatePosSections = true;
}
