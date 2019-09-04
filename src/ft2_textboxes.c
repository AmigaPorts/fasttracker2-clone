/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_video.h"
#include "ft2_config.h"
#include "ft2_nibbles.h"
#include "ft2_diskop.h"
#include "ft2_sample_ed.h"
#include "ft2_pattern_ed.h"
#include "ft2_keyboard.h"
#include "ft2_gfxdata.h"
#include "ft2_mouse.h"
#include "ft2_edit.h"

static int16_t markX1, markX2;
static uint16_t oldCursorPos, oldMouseX;

textBox_t textBoxes[NUM_TEXTBOXES] =
{
    // tx = how much to add to x (for text)
    // ty = how much to add to y (for text)
    // tw = how much to render of text in pixels
    // maxChars = max characters in box

    /* -- instrument switcher and song name elements must be in this order! --------- */

    // instrument switcher
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  446,   5, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  16, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  27, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  38, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  49, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  60, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  71, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446,  82, 139,  10, 1, 0, 138, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },

    // sample switcher
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  446,  99, 115,  10, 1, 0, 114, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446, 110, 115,  10, 1, 0, 114, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446, 121, 115,  10, 1, 0, 114, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446, 132, 115,  10, 1, 0, 114, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },
    {  446, 143, 115,  10, 1, 0, 114, 22,       true, NULL, false, 0, NULL, 0, 0, 0, 0 },

    // song name
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  424, 158, 160,  12, 2, 1, 156, 20,       false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    /* -- you can hereby change the structure ----------------------------------------*/

    // nibbles - player 1 and 2 name
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  215, 273, 203,  12, 2, 1, 199, 22,       false, NULL, true, 0, NULL, 0, 0, 0, 0 },
    {  215, 273, 203,  12, 2, 1, 199, 22,       false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // config - default directories
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    { 486,  16, 143,  12, 2, 1, 139, 80,        false, NULL, true, 0, NULL, 0, 0, 0, 0 },
    { 486,  31, 143,  12, 2, 1, 139, 80,        false, NULL, true, 0, NULL, 0, 0, 0, 0 },
    { 486,  46, 143,  12, 2, 1, 139, 80,        false, NULL, true, 0, NULL, 0, 0, 0, 0 },
    { 486,  61, 143,  12, 2, 1, 139, 80,        false, NULL, true, 0, NULL, 0, 0, 0, 0 },
    { 486,  76, 143,  12, 2, 1, 139, 80,        false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // disk op. filename
    // x,   y,   w,   h,  tx,ty, tw,  maxChars,  rightMouseButton
    {   62, 158, 103,  12, 2, 1, 99, PATH_MAX-1, false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // disk op. rename name
    // x,   y,   w,   h,  tx,ty, tw,  maxChars,   rightMouseButton
    {  177, 273, 278,  12, 2, 1, 274, PATH_MAX-1, false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // disk op. new directory name
    // x,   y,   w,   h,  tx,ty, tw,  maxChars,   rightMouseButton
    {  177, 273, 278,  12, 2, 1, 274, PATH_MAX-1, false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // disk op. set path name
    // x,   y,   w,   h,  tx,ty, tw,  maxChars,   rightMouseButton
    {  177, 273, 278,  12, 2, 1, 274, PATH_MAX-1, false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // save range filename
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  215, 273, 203,  12, 2, 1, 199, 128-1,    false, NULL, true, 0, NULL, 0, 0, 0, 0 },

    // scale fade volume
    // x,   y,   w,   h,  tx,ty, tw,  maxChars, rightMouseButton
    {  215, 273, 203,  12, 2, 1, 199, 32,       false, NULL, true, 0, NULL, 0, 0, 0, 0 },
};

static void moveTextCursorLeft(int16_t i, uint8_t updateTextBox);
static void moveTextCursorRight(int16_t i, uint8_t updateTextBox);

static void setSongModifiedFlagIfNeeded(void) /* called during keystrokes in text boxes */
{
    if ((mouse.lastEditBox == TB_SONG_NAME) ||
       ((mouse.lastEditBox >= TB_INST1) && (mouse.lastEditBox <= TB_INST8)) ||
       ((mouse.lastEditBox >= TB_SAMP1) && (mouse.lastEditBox <= TB_SAMP5))
       )
    {
        setSongModifiedFlag();
    }
}

int8_t textIsMarked(void)
{
    if (markX1 == markX2)
        return (false);

    return (true);
}

static void removeTextMarking(void)
{
    markX1 = 0;
    markX2 = 0;
}

static int16_t getTextMarkStart(void)
{
    if (markX2 < markX1)
        return (markX2);

    return (markX1);
}

static int16_t getTextMarkEnd(void)
{
    if (markX1 > markX2)
        return (markX1);

    return (markX2);
}

static int16_t getTextLength(textBox_t *t, uint32_t offset)
{
    int16_t numChars;

    /* count number of characters in text (don't use strlen() - not safe here!) */
    for (numChars = 0; numChars < t->maxChars; ++numChars)
    {
        if (((offset + numChars) >= t->maxChars) || (t->textPtr[offset + numChars] == '\0'))
            break;
    }

    return (numChars);
}

static void deleteMarkedText(textBox_t *t)
{
    int16_t start, end;
    int32_t i, deleteTextWidth, length;

    if (!textIsMarked())
        return;

    start = getTextMarkStart();
    end   = getTextMarkEnd();

    MY_ASSERT((start < t->maxChars) && (end <= t->maxChars))

    /* calculate pixel width of string to delete */
    deleteTextWidth = 0;
    for (i = start; i < end; ++i)
        deleteTextWidth += getCharWidth(t->textPtr[i], FONT_TYPE1);

    /* copy markEnd part to markStart, and zero out leftover junk */
    length = (int32_t)(strlen(&t->textPtr[end]));
    memcpy(&t->textPtr[start], &t->textPtr[end], length);
    memset(&t->textPtr[start + length], 0, t->maxChars - (start + length));

    /* move buffer offset to the left if scrolled */
    if (t->bufOffset >= deleteTextWidth)
        t->bufOffset -= deleteTextWidth;
    else
        t->bufOffset = 0;

    /* set text cursor to markStart */
    t->cursorPos = start;

    setSongModifiedFlagIfNeeded();
}

static void setCursorToMarkStart(textBox_t *t)
{
    char ch;
    int16_t start;
    int32_t startXPos, i;

    if (!textIsMarked())
        return;

    start = getTextMarkStart();

    MY_ASSERT(start < t->maxChars)

    t->cursorPos = start;

    startXPos = 0;
    for (i = 0; i < start; ++i)
    {
        ch = t->textPtr[i];
        if (ch == '\0')
            break;

        startXPos += getCharWidth(ch, FONT_TYPE1);
    }

    /* change buffer offset, if needed */
    if (startXPos < t->bufOffset)
        t->bufOffset = startXPos;
}

static void setCursorToMarkEnd(textBox_t *t)
{
    char ch;
    int16_t end;
    int32_t endXPos, i;

    if (!textIsMarked())
        return;

    end = getTextMarkEnd();

    MY_ASSERT(end <= t->maxChars)

    t->cursorPos = end;

    endXPos = 0;
    for (i = 0; i < end; ++i)
    {
        ch = t->textPtr[i];
        if (ch == '\0')
            break;

        endXPos += getCharWidth(ch, FONT_TYPE1);
    }

    /* change buffer offset, if needed */
    if (endXPos > (t->bufOffset + t->tw))
        t->bufOffset = endXPos - t->tw;
}

static void copyMarkedText(textBox_t *t)
{
    int32_t length, start, end;
    char *utf8Text;

    if (!textIsMarked())
        return;

    start = getTextMarkStart();
    end   = getTextMarkEnd();

    MY_ASSERT((start < t->maxChars) && (end <= t->maxChars))

    length = end - start;
    if (length < 1)
        return;

    utf8Text = cp437ToUtf8(&t->textPtr[start]);
    if (utf8Text != NULL)
    {
        SDL_SetClipboardText(utf8Text);
        free(utf8Text);
    }
}

static void cutMarkedText(textBox_t *t)
{
    if (!textIsMarked())
        return;

    copyMarkedText(t);
    deleteMarkedText(t);
    removeTextMarking();

    drawTextBox(mouse.lastEditBox);
}

static void pasteText(textBox_t *t)
{
    char *copiedText, *copiedTextUtf8, *endPart;
    int32_t i, textLength, roomLeft, copiedTextLength, endOffset, endPartLength;

    if (!SDL_HasClipboardText())
        return;

    /* if we've marked text, delete it and remove text marking */
    if (textIsMarked())
    {
        deleteMarkedText(t);
        removeTextMarking();
    }

    if (t->cursorPos >= t->maxChars)
        return;

    textLength = getTextLength(t, 0);

    roomLeft = t->maxChars - textLength;
    if (roomLeft <= 0)
        return; /* no more room! */
 
    copiedTextUtf8 = SDL_GetClipboardText();

    copiedText = utf8ToCp437(copiedTextUtf8, true);
    if (copiedText == NULL)
        return;

    copiedTextLength = (int32_t)(strlen(copiedText));
    if (copiedTextLength > roomLeft)
        copiedTextLength = roomLeft;

    endOffset = t->cursorPos;
    endPart   = NULL; /* prevent false compiler warning */

    endPartLength = getTextLength(t, endOffset);
    if (endPartLength > 0)
    {
        endPart = (char *)(malloc(endPartLength + 1));
        if (endPart == NULL)
        {
            free(copiedText);
            sysReqQueue(SR_OOM_ERROR);
            return;
        }
    }

    /* make a copy of end data */
    if (endPartLength > 0)
    {
        memcpy(endPart, &t->textPtr[endOffset], endPartLength);
        endPart[endPartLength] = '\0';
    }

    /* paste copied data */
    memcpy(&t->textPtr[endOffset], copiedText, copiedTextLength);
    t->textPtr[endOffset + copiedTextLength] = '\0';
    free(copiedText);

    /* append end data */
    if (endPartLength > 0)
    {
        strcat(&t->textPtr[endOffset + copiedTextLength], endPart);
        free(endPart);
    }

    for (i = 0; i < copiedTextLength; ++i)
        moveTextCursorRight(mouse.lastEditBox, TEXTBOX_NO_UPDATE);

    drawTextBox(mouse.lastEditBox);

    setSongModifiedFlagIfNeeded();
}

static void blitClipW(uint16_t xPos, uint16_t yPos, const uint8_t *srcPtr, uint32_t w, uint16_t h, uint32_t clipW)
{
    uint32_t *dstPtr, x, y, blitW;

    blitW = w;
    if (blitW > clipW)
        blitW = clipW;

    MY_ASSERT((xPos < SCREEN_W) && (yPos < SCREEN_H) && ((xPos + blitW) <= SCREEN_W) &&
             ((yPos + h) <= SCREEN_H) && (srcPtr != NULL))

    dstPtr = &video.frameBuffer[(yPos * SCREEN_W) + xPos];
    for (y = 0; y < h; ++y)
    {
        for (x = 0; x < blitW; ++x)
        {
            if (srcPtr[x] != PAL_TRANSPR)
                dstPtr[x] = video.palette[srcPtr[x]];
        }

        srcPtr += w;
        dstPtr += SCREEN_W;
    }
}

static void charOutBuf(uint8_t *dstBuffer, uint32_t dstWidth, uint32_t xPos, uint32_t yPos, uint8_t paletteIndex, char chr)
{
    uint8_t *dstPtr, x, y;
    const uint8_t *srcPtr;

    if ((chr == '\0') || (chr == ' '))
        return;

    srcPtr = &font1Data[chr * FONT1_CHAR_W];
    dstPtr = &dstBuffer[(yPos * dstWidth) + xPos];

    for (y = 0; y < FONT1_CHAR_H; ++y)
    {
        for (x = 0; x < FONT1_CHAR_W; ++x)
        {
            if (srcPtr[x])
                dstPtr[x] = paletteIndex;
        }

        srcPtr += FONT1_WIDTH;
        dstPtr += dstWidth;
    }
}

static void textOutBuf(uint8_t *dstBuffer, uint32_t dstWidth, uint16_t x, uint16_t y, uint8_t paletteIndex, char *text, uint32_t textLength)
{
    char ch;
    uint16_t currX;
    uint32_t i;

    MY_ASSERT(text != NULL)

    if (*text == '\0')
        return;

    currX = 0;
    for (i = 0; i < textLength; ++i)
    {
        ch = relocateChars(text[i], FONT_TYPE1);

        charOutBuf(dstBuffer, dstWidth, x + currX, y, paletteIndex, ch);
        currX += getCharWidth(ch, FONT_TYPE1);
    }
}

void exitTextEditing(void)
{
    if (editor.ui.editTextFlag)
    {
        if ((mouse.lastEditBox >= 0) && (mouse.lastEditBox < NUM_TEXTBOXES))
        {
            textBoxes[mouse.lastEditBox].bufOffset = 0;

            removeTextMarking();
            drawTextBox(mouse.lastEditBox);
        }

        keyb.ignoreCurrKeyUp   = true; /* prevent "note off" */
        editor.ui.editTextFlag = false;

        if ((mouse.lastEditBox == TB_DISKOP_FILENAME) && (getDiskOpItem() == DISKOP_ITEM_MODULE))
        {
            updateCurrSongFilename(); /* for window title */
            updateWindowTitle(true);
        }
    }

    hideSprite(SPRITE_TEXT_CURSOR);
}

static inline int16_t cursorPosToX(textBox_t *t)
{
    int16_t i;
    int32_t x;

    MY_ASSERT(t->textPtr != NULL)

    x = -1; /* cursor starts one pixel before character */
    for (i = 0; i < t->cursorPos; ++i)
        x += getCharWidth(t->textPtr[i], FONT_TYPE1);

    /* subtract by buffer offset to get real X position */
    x -= t->bufOffset;

    return ((int16_t)(x));
}

int16_t getTextCursorX(textBox_t *t)
{
    return (t->x + t->tx + cursorPosToX(t));
}

int16_t getTextCursorY(textBox_t *t)
{
    return (t->y + t->ty);
}

static void scrollTextBufferLeft(textBox_t *t)
{
    /* scroll buffer and clamp */
    t->bufOffset -= TEXT_SCROLL_VALUE;
    if (t->bufOffset < 0)
        t->bufOffset = 0;
}

static void scrollTextBufferRight(textBox_t *t, uint16_t numCharsInText)
{
    uint16_t j;
    int32_t textEnd;

    MY_ASSERT(numCharsInText <= t->maxChars)

    /* get end of text position */
    textEnd = 0;
    for (j = 0; j < numCharsInText; ++j)
        textEnd += getCharWidth(t->textPtr[j], FONT_TYPE1);

    /* subtract by text box width and clamp to 0 */
    textEnd -= t->tw;
    if (textEnd < 0)
        textEnd = 0;

    /* scroll buffer and clamp */
    t->bufOffset += TEXT_SCROLL_VALUE;
    if (t->bufOffset > textEnd)
        t->bufOffset = textEnd;
}

static void moveTextCursorToMouseX(uint16_t textBoxID)
{
    int8_t cw;
    int16_t i, numChars, cursorPos;
    int32_t mx, tx, tx2;
    textBox_t *t;

    t = &textBoxes[textBoxID];
    if (((mouse.x == t->x) && (t->bufOffset == 0)) || (t->textPtr == NULL) || (t->textPtr[0] == '\0'))
    {
        t->cursorPos = 0;
        return;
    }

    numChars = getTextLength(t, 0);

    /* find out what character we are clicking at, and set cursor to that character */
    mx = t->bufOffset + mouse.x;
    tx = (t->x + t->tx) - 1;
    cw = -1;

    for (i = 0; i < numChars; ++i)
    {
        cw  = getCharWidth(t->textPtr[i], FONT_TYPE1);
        tx2 = tx + cw;

        if ((mx >= tx) && (mx < tx2))
        {
            t->cursorPos = i;
            break;
        }

        tx += cw;
    }

    /* set to last character if we clicked outside the end of the text */
    if ((i == numChars) && (mx >= tx))
        t->cursorPos = numChars;

    if (cw != -1)
    {
        cursorPos = cursorPosToX(t);

        /* scroll buffer to the right if needed */
        if ((cursorPos + cw) > t->tw)
            scrollTextBufferRight(t, numChars);

        /* scroll buffer to the left if needed */
        else if (cursorPos < (0 - 1))
            scrollTextBufferLeft(t);
    }

    editor.textCursorBlinkCounter = 0;
}

void drawTextBox(uint16_t textBoxID)
{
    char ch;
    int8_t cw;
    uint8_t pal;
    uint16_t i, y;
    int32_t start, end, x1, x2, length;
    textBox_t *t;

    MY_ASSERT(textBoxID < NUM_TEXTBOXES)

    t = &textBoxes[textBoxID];
    if (!t->visible)
        return;

    memset(t->renderBuf, PAL_TRANSPR, t->renderBufWidth * t->renderBufHeight);

    /* draw text mark background */
    if (textIsMarked())
    {
        hideSprite(SPRITE_TEXT_CURSOR);

        start = getTextMarkStart();
        end   = getTextMarkEnd();

        MY_ASSERT((start < t->maxChars) && (end <= t->maxChars))

        /* find pixel start/length from markX1 and markX2 */

        x1 = 0; x2 = 0;
        for (i = 0; i < end; ++i)
        {
            ch = t->textPtr[i];
            if (ch == '\0')
                break;

            cw = getCharWidth(ch, FONT_TYPE1);
            if (i < start)
                x1 += cw;

            x2 += cw;
        }

        /* render text mark background */
        if (x1 != x2)
        {
            start  = x1;
            length = x2 - x1;

            MY_ASSERT((start + length) <= t->renderBufWidth)

            for (y = 0; y < t->renderBufHeight; ++y)
                memset(&t->renderBuf[(y * t->renderBufWidth) + start], PAL_TEXTMRK, length);
        }
    }

    textOutBuf(t->renderBuf, t->renderBufWidth, 0, 0, PAL_FORGRND, t->textPtr, t->maxChars);

    pal = video.frameBuffer[(t->y * SCREEN_W) + t->x] >> 24; /* get background palette (stored in alpha channel) */
    fillRect(t->x + t->tx, t->y + t->ty, t->tw, 10, pal);

    MY_ASSERT(t->bufOffset <= (t->renderBufWidth - t->tw))

    blitClipW(t->x + t->tx, t->y + t->ty, t->renderBuf + t->bufOffset, t->renderBufWidth, t->renderBufHeight, t->tw);
}

void showTextBox(uint16_t textBoxID)
{
    MY_ASSERT(textBoxID < NUM_TEXTBOXES)

    textBoxes[textBoxID].visible = true;
}

void hideTextBox(uint16_t textBoxID)
{
    MY_ASSERT(textBoxID < NUM_TEXTBOXES)

    hideSprite(SPRITE_TEXT_CURSOR);
    textBoxes[textBoxID].visible = false;
}

void setupTextBoxForSysReq(int16_t textBoxID, char *textPtr, int16_t textPtrLen, uint8_t setCursorToEnd)
{
    int16_t textW, textLen;
    textBox_t *t;

    MY_ASSERT((textBoxID >= 0) && (textBoxID < NUM_TEXTBOXES) && (textPtr != NULL))

    t = &textBoxes[textBoxID];

    t->visible   = false; /* will be shown later */
    t->textPtr   = textPtr;
    t->cursorPos = 0;
    t->bufOffset = 0;

    if (setCursorToEnd)
    {
        for (textLen = 0; textLen < textPtrLen; ++textLen)
        {
            if (textPtr[textLen] == '\0')
                break;
        }

        t->cursorPos = textLen;

        textW = getTextWidth(textPtr, FONT_TYPE1);
        if (textW >= t->tw)
            t->bufOffset = (int16_t)(textW - t->tw);
    } 
}

static void setMarkX2ToMouseX(textBox_t *t)
{
    int8_t cw;
    int16_t i, numChars;
    int32_t mx, tx, tx2;

    if ((t->textPtr == NULL) || (t->textPtr[0] == '\0'))
    {
        removeTextMarking();
        return;
    }

    if ((markX2 < markX1) && (mouse.x < (t->x + t->tx)))
    {
        markX2 = 0;
        return;
    }

    numChars = getTextLength(t, 0);

    /* find out what character we are clicking at, and set markX2 to that character */
    mx = t->bufOffset + mouse.x;
    tx = (t->x + t->tx) - 1;

    for (i = 0; i < numChars; ++i)
    {
        cw  = getCharWidth(t->textPtr[i], FONT_TYPE1);
        tx2 = tx + cw;

        if ((mx >= tx) && (mx < tx2))
        {
            markX2 = i;
            break;
        }

        tx += cw;
    }

    /* set to last character if we clicked outside the end of the text */
    if ((i == numChars) && (mx >= tx))
        markX2 = numChars;

    if (mouse.x >= ((t->x + t->w) - 3))
    {
        scrollTextBufferRight(t, numChars);
        if (++markX2 > numChars)
              markX2 = numChars;
    }
    else if (mouse.x <= ((t->x + t->tx) + 3))
    {
        if (t->bufOffset > 0)
        {
            scrollTextBufferLeft(t);
            if (--markX2 < 0)
                  markX2 = 0;
        }
    }

    t->cursorPos = markX2;

    MY_ASSERT((t->cursorPos >= 0) && (t->cursorPos <= getTextLength(t, 0)))

    editor.textCursorBlinkCounter = 0;
}

void handleTextBoxWhileMouseDown(void)
{
    textBox_t *t;

    MY_ASSERT((mouse.lastUsedObjectID >= 0) && (mouse.lastUsedObjectID < NUM_TEXTBOXES))

    t = &textBoxes[mouse.lastUsedObjectID];
    if (!t->visible)
        return;

    if (mouse.x != oldMouseX)
    {
        oldMouseX = mouse.x;

        markX1 = oldCursorPos;
        setMarkX2ToMouseX(t);

        drawTextBox(mouse.lastUsedObjectID);
    }
}

int8_t testTextBoxMouseDown(void)
{
    uint16_t i;
    textBox_t *t;

    oldMouseX = mouse.x;
    oldCursorPos = 0;

    for (i = 0; i < NUM_TEXTBOXES; ++i)
    {
        t = &textBoxes[i];
        if (!t->visible)
            continue;

        /* if a system request is shown, only handle textboxes within a system request */
        if (editor.ui.systemRequestShown)
        {
            switch (i)
            {
                default: continue;
                case TB_NIB_PLAYER1_NAME:
                case TB_NIB_PLAYER2_NAME:
                case TB_DISKOP_RENAME_NAME:
                case TB_DISKOP_MAKEDIR_NAME:
                case TB_DISKOP_SETPATH_NAME:
                case TB_SCALE_FADE_VOL:
                break;
            }
        }

        if ((mouse.y >= t->y) && (mouse.y < (t->y + t->h)))
        {
            if ((mouse.x >= t->x) && (mouse.x < (t->x + t->w)))
            {
                if (!mouse.rightButtonPressed && t->rightMouseButton)
                    break;

                /* if we were editing another text box and clicked on another one, properly end it */
                if (editor.ui.editTextFlag && (i != mouse.lastEditBox))
                    exitTextEditing();

                mouse.lastEditBox = i;
                moveTextCursorToMouseX(mouse.lastEditBox);

                oldCursorPos = t->cursorPos;
                removeTextMarking();
                drawTextBox(mouse.lastEditBox);

                editor.textCursorBlinkCounter  = 0;
                mouse.lastUsedObjectType = OBJECT_TEXTBOX;
                mouse.lastUsedObjectID   = i;

                editor.ui.editTextFlag = true;
                return (true);
            }
        }
    }

    /* if we were editing text and we clicked outside of a text box, exit text editing */
    if (editor.ui.editTextFlag && !editor.ui.systemRequestShown)
        exitTextEditing();

    return (false);
}

void updateTextBoxPointers(void)
{
    uint8_t i;
    instrTyp *curIns;

    curIns = &instr[editor.curInstr];

    /* instrument names */
    for (i = 0; i < 8; ++i)
        textBoxes[TB_INST1 + i].textPtr = song.instrName[1 + editor.instrBankOffset + i];

    /* sample names */
    for (i = 0; i < 5; ++i)
        textBoxes[TB_SAMP1 + i].textPtr = curIns->samp[editor.sampleBankOffset + i].name;

    /* song name */
    textBoxes[TB_SONG_NAME].textPtr = song.name;
}

void setupInitialTextBoxPointers(void)
{
    textBoxes[TB_CONF_DEF_MODS_DIR].textPtr   = &config.modulesPath[1];
    textBoxes[TB_CONF_DEF_INSTRS_DIR].textPtr = &config.instrPath[1];
    textBoxes[TB_CONF_DEF_SAMPS_DIR].textPtr  = &config.samplesPath[1];
    textBoxes[TB_CONF_DEF_PATTS_DIR].textPtr  = &config.patternsPath[1];
    textBoxes[TB_CONF_DEF_TRACKS_DIR].textPtr = &config.tracksPath[1];
}

void handleTextEditControl(SDL_Keycode keycode)
{
    char ch;
    int16_t i;
    uint16_t numChars;
    int32_t textLength;
    uint32_t textWidth;
    textBox_t *t;

    MY_ASSERT((mouse.lastEditBox >= 0) && (mouse.lastEditBox < NUM_TEXTBOXES))

    t = &textBoxes[mouse.lastEditBox];

    MY_ASSERT(t->textPtr != NULL)

    switch (keycode)
    {
        case SDLK_ESCAPE:
        {
            removeTextMarking();

            if (mouse.lastEditBox == TB_NIB_PLAYER1_NAME)
                nibblesPlayer1NameOK();
            else if (mouse.lastEditBox == TB_NIB_PLAYER2_NAME)
                nibblesPlayer2NameOK();
            else
                exitTextEditing();

            if (editor.ui.systemRequestShown)
                hideSystemRequest();
        }
        break;

        case SDLK_a:
        {
            /* CTRL+A - mark all text */
            if (keyb.ctrlPressed || keyb.commandPressed)
            {
                /* count number of chars and get full text width */
                textWidth = 0;
                for (numChars = 0; numChars < t->maxChars; ++numChars)
                {
                    if (t->textPtr[numChars] == '\0')
                        break;

                    textWidth += getCharWidth(t->textPtr[numChars], FONT_TYPE1);
                }

                markX1 = 0;
                markX2 = numChars;
                t->cursorPos = markX2;

                t->bufOffset = (textWidth > t->tw) ? (textWidth - t->tw) : 0;

                drawTextBox(mouse.lastEditBox);
            }
        }
        break;

        case SDLK_x:
        {
            /* CTRL+X - cut marked text */
            if (keyb.ctrlPressed || keyb.commandPressed)
                cutMarkedText(t);
        }
        break;

        case SDLK_c:
        {
            /* CTRL+C - copy marked text */
            if (keyb.ctrlPressed || keyb.commandPressed)
                copyMarkedText(t);
        }
        break;

        case SDLK_v:
        {
            /* CTRL+V - paste text */
            if (keyb.ctrlPressed || keyb.commandPressed)
                pasteText(t);
        }
        break;

        case SDLK_KP_ENTER:
        case SDLK_RETURN:
        {
            /* handle RETURN/ENTER event for textboxes ( hardcoded :( ) */

            if (keyb.leftAltPressed)
                toggleFullScreen();
            else if (mouse.lastEditBox == TB_NIB_PLAYER1_NAME)
                nibblesPlayer1NameOK();
            else if (mouse.lastEditBox == TB_NIB_PLAYER2_NAME)
                nibblesPlayer2NameOK();
            else if (mouse.lastEditBox == TB_DISKOP_RENAME_NAME)
                diskOpRenameAnsi();
            else if (mouse.lastEditBox == TB_DISKOP_MAKEDIR_NAME)
                diskOpMakeDirAnsi();
            else if (mouse.lastEditBox == TB_DISKOP_SETPATH_NAME)
                diskOpSetPathAnsi();
            else if (mouse.lastEditBox == TB_SCALE_FADE_VOL)
                handleScaleFadeVolume();
            else if (mouse.lastEditBox == TB_SAVE_RANGE_FILENAME)
                saveRange2();
            else
                exitTextEditing();
        }
        break;

        case SDLK_LEFT:
        {
            if (keyb.leftShiftPressed)
            {
                if (!textIsMarked())
                {
                    /* no marking, mark character to left from cursor */
                    if (t->cursorPos > 0)
                    {
                        markX1 = t->cursorPos;
                        moveTextCursorLeft(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                        markX2 = t->cursorPos;

                        drawTextBox(mouse.lastEditBox);
                    }
                }
                else
                {
                    /* marking, extend/shrink marking */
                    if (markX2 > 0)
                    {
                        t->cursorPos = markX2;
                        markX2--;
                        moveTextCursorLeft(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                        drawTextBox(mouse.lastEditBox);
                    }
                }
            }
            else
            {
                if (textIsMarked())
                {
                    setCursorToMarkStart(t);
                    removeTextMarking();
                }
                else
                {
                    removeTextMarking();
                    moveTextCursorLeft(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                }

                drawTextBox(mouse.lastEditBox);
            }
        }
        break;

        case SDLK_RIGHT:
        {
            if (keyb.leftShiftPressed)
            {
                textLength = getTextLength(t, 0);

                if (!textIsMarked())
                {
                    if (t->cursorPos < textLength)
                    {
                        markX1 = t->cursorPos;

                        moveTextCursorRight(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                        markX2 = t->cursorPos;

                        drawTextBox(mouse.lastEditBox);
                    }
                }
                else
                {
                    /* marking, extend/shrink marking */
                    if (markX2 < textLength)
                    {
                        t->cursorPos = markX2;
                        markX2++;
                        moveTextCursorRight(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                        drawTextBox(mouse.lastEditBox);
                    }
                }
            }
            else
            {
                if (textIsMarked())
                {
                    setCursorToMarkEnd(t);
                    removeTextMarking();
                }
                else
                {
                    removeTextMarking();
                    moveTextCursorRight(mouse.lastEditBox, TEXTBOX_NO_UPDATE);
                }

                drawTextBox(mouse.lastEditBox);
            }
        }
        break;

        case SDLK_BACKSPACE:
        {
            if (textIsMarked())
            {
                deleteMarkedText(t);
                removeTextMarking();
                drawTextBox(mouse.lastEditBox);
                break;
            }

            removeTextMarking();

            if ((t->cursorPos > 0) && (t->textPtr[0] != '\0'))
            {
                if (t->bufOffset > 0)
                {
                    ch = relocateChars(t->textPtr[t->cursorPos - 1], FONT_TYPE1);

                    t->bufOffset -= font1Widths[(uint32_t)(ch)];
                    if (t->bufOffset < 0)
                        t->bufOffset = 0;
                }

                moveTextCursorLeft(mouse.lastEditBox, TEXTBOX_UPDATE);

                for (i = t->cursorPos; i < (t->maxChars - 1); ++i)
                    t->textPtr[i] = t->textPtr[i + 1];
                t->textPtr[t->maxChars - 1] = '\0';

                drawTextBox(mouse.lastEditBox);
                setSongModifiedFlagIfNeeded();
            }
        }
        break;

        case SDLK_DELETE:
        {
            if (textIsMarked())
            {
                deleteMarkedText(t);
                removeTextMarking();
                drawTextBox(mouse.lastEditBox);
                break;
            }

            if ((t->textPtr[t->cursorPos] != '\0') && (t->textPtr[0] != '\0') && (t->cursorPos < t->maxChars))
            {
                if (t->bufOffset > 0) /* are we scrolled? */
                {
                    ch = relocateChars(t->textPtr[t->cursorPos], FONT_TYPE1);

                    t->bufOffset -= font1Widths[(uint32_t)(ch)];
                    if (t->bufOffset < 0)
                        t->bufOffset = 0;
                }

                for (i = t->cursorPos; i < (t->maxChars - 1); ++i)
                    t->textPtr[i] = t->textPtr[i + 1];
                t->textPtr[t->maxChars - 1] = '\0';

                drawTextBox(mouse.lastEditBox);
                setSongModifiedFlagIfNeeded();
            }
        }
        break;

        case SDLK_HOME:
        {
            if (keyb.leftShiftPressed)
            {
                if (!textIsMarked())
                    markX1 = t->cursorPos;

                markX2 = 0;
                t->bufOffset = 0;
                t->cursorPos = 0;
            }
            else
            {
                removeTextMarking();

                if (t->cursorPos > 0)
                {
                    t->cursorPos = 0;
                    t->bufOffset = 0;

                    editor.textCursorBlinkCounter = 0;
                }
            }

            drawTextBox(mouse.lastEditBox);
        }
        break;

        case SDLK_END:
        {
            /* count number of chars and get full text width */
            textWidth = 0;
            for (numChars = 0; numChars < t->maxChars; ++numChars)
            {
                if (t->textPtr[numChars] == '\0')
                    break;

                textWidth += getCharWidth(t->textPtr[numChars], FONT_TYPE1);
            }

            if (t->cursorPos < numChars)
            {
                if (keyb.leftShiftPressed)
                {
                    if (!textIsMarked())
                        markX1 = t->cursorPos;

                    markX2 = numChars;
                }
                else
                {
                    removeTextMarking();
                }

                t->cursorPos = numChars;
                t->bufOffset = (textWidth > t->tw) ? (textWidth - t->tw) : 0;

                drawTextBox(mouse.lastEditBox);
                editor.textCursorBlinkCounter = 0;
            }
        }
        break;

        default: break;
    }
}

void handleTextEditInputChar(char textChar)
{
    int16_t i;
    textBox_t *t;

    MY_ASSERT((mouse.lastEditBox >= 0) && (mouse.lastEditBox < NUM_TEXTBOXES))

    t = &textBoxes[mouse.lastEditBox];

    MY_ASSERT(t->textPtr != NULL)

    /* only certain negative values are allowed! */
    if (textChar < ' ')
    {
        /* allow certain codepage 437 nordic characters */
        if ((textChar != -124) && (textChar != -108) && (textChar != -122) &&
            (textChar != -114) && (textChar != -103) && (textChar != -113))
        {
            return;
        }
    }

    /* this text box can only handle certain characters */
    if (mouse.lastEditBox == TB_SCALE_FADE_VOL)
    {
        if (((textChar != '.') && (textChar != ',')) && ((textChar < '0') || (textChar > '9')))
            return;
    }

    if (textIsMarked())
    {
        deleteMarkedText(t);
        removeTextMarking();
    }

    if ((t->cursorPos >= 0) && (t->cursorPos < t->maxChars))
    {
        i = getTextLength(t, 0);
        if (i < t->maxChars) /* do we have room for a new character? */
        {
            for (i = (t->maxChars - 1); i > t->cursorPos; --i)
                t->textPtr[i] = t->textPtr[i - 1];
            t->textPtr[t->cursorPos] = textChar;

            moveTextCursorRight(mouse.lastEditBox, TEXTBOX_UPDATE); /* also updates textbox */
            setSongModifiedFlagIfNeeded();
        }
    }
}

static void moveTextCursorLeft(int16_t i, uint8_t updateTextBox)
{
    textBox_t *t;

    t = &textBoxes[i];
    if (t->cursorPos > 0)
    {
        t->cursorPos--;

        /* scroll buffer if needed */
        if (cursorPosToX(t) < (0 - 1))
            scrollTextBufferLeft(t);

        if (updateTextBox)
            drawTextBox(i);

        editor.textCursorBlinkCounter = 0; /* reset text cursor blink timer */
    }
}

static void moveTextCursorRight(int16_t i, uint8_t updateTextBox)
{
    uint16_t numChars;
    textBox_t *t;

    t = &textBoxes[i];

    numChars = getTextLength(t, 0);
    if (t->cursorPos < numChars)
    {
        t->cursorPos++;

        /* scroll buffer if needed */
        if (cursorPosToX(t) >= t->tw)
            scrollTextBufferRight(t, numChars);

        if (updateTextBox)
            drawTextBox(i);

        editor.textCursorBlinkCounter = 0; /* reset text cursor blink timer */
    }
}

void freeTextBoxes(void)
{
    int32_t i;
    textBox_t *t;

    /* free up text edit box buffers */
    for (i = 0; i < NUM_TEXTBOXES; ++i)
    {
        t = &textBoxes[i];
        if (t->renderBuf != NULL)
        {
            free(t->renderBuf);
            t->renderBuf = NULL;
        }
    }
}
