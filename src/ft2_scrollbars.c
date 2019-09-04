/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include "ft2_header.h"
#include "ft2_gui.h"
#include "ft2_config.h"
#include "ft2_audio.h"
#include "ft2_help.h"
#include "ft2_sample_ed.h"
#include "ft2_inst_ed.h"
#include "ft2_diskop.h"
#include "ft2_pattern_ed.h"
#include "ft2_audioselector.h"
#include "ft2_midi.h"
#include "ft2_mouse.h"
#include "ft2_video.h"

scrollBar_t scrollBars[NUM_SCROLLBARS] =
{
    // ------ POSITION EDITOR SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    {  55,  15,  18,  21, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbPosEdPos },

    // ------ INSTRUMENT SWITCHER SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 566, 112,  18,  28, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbSmpBankPos },

    // ------ PATTERN VIEWER SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    {  28, 385, 576,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   setChannelScrollPos },

    // ------ HELP SCREEN SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 611,  15,  18, 143, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   helpScrollSetPos },

    // ------ SAMPLE EDITOR SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    {  26, 331, 580,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   scrollSampleData },

    // ------ INSTRUMENT EDITOR SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 544, 175,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setVolumeScroll },
    { 544, 189,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setPanningScroll },
    { 544, 203,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setFinetuneScroll },
    { 544, 220,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setFadeoutScroll },
    { 544, 234,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setVibSpeedScroll },
    { 544, 248,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setVibDepthScroll },
    { 544, 262,  62,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, setVibSweepScroll },

    // ------ INSTRUMENT EDITOR EXTENSION SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 195, 130,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbMidiChPos },
    { 195, 144,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbMidiPrgPos },
    { 195, 158,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbMidiBendPos },

    // ------ CONFIG AUDIO SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 365,  29,  18,  43, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbAudOutputSetPos },
    { 365, 116,  18,  42, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbAudInputSetPos },
    { 529, 132,  79,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbAmp },
    { 529, 158,  79,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbMasterVol },

    // ------ CONFIG LAYOUT SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 536,  15,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbPalRPos },
    { 536,  29,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbPalGPos },
    { 536,  43,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbPalBPos },
    { 536,  71,  70,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbPalContrastPos },

    // ------ CONFIG MISCELLANEOUS SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 578, 158,  29,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbMIDISens },

    // ------ CONFIG MIDI SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 483,  15,  18, 143, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbMidiInputSetPos },

    // ------ DISK OP. SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 335,  15,  18, 143, false,   SCROLLBAR_VERTICAL,   SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_FLAT,   sbDiskOpSetPos },

    // ------ SAMPLE VOLUME BOX SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 315, 234, 124,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetStartVolPos },
    { 315, 248, 124,  13, false,   SCROLLBAR_HORIZONTAL, SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetEndVolPos   },

    // ------ SAMPLE RESAMPLE BOX SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 337, 234, 58,  13, false,   SCROLLBAR_HORIZONTAL,  SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetResampleTones },

    // ------ SAMPLE MIX SAMPLE BOX SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 345, 244, 66,  13, false,   SCROLLBAR_HORIZONTAL,  SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetMixBalancePos },

    // ------ SAMPLE ECHO BOX SCROLLBARS ------
    //x,   y,   w,   h,   visible, type,                 state,                              style                   funcOnDown
    { 368, 224, 66,  13, false,   SCROLLBAR_HORIZONTAL,  SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetEchoNumPos },
    { 368, 238, 66,  13, false,   SCROLLBAR_HORIZONTAL,  SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetEchoDistPos },
    { 368, 252, 66,  13, false,   SCROLLBAR_HORIZONTAL,  SCROLLBAR_UNPRESSED, 0,0,0,0,0,0,0, SCROLLBAR_THUMB_NOFLAT, sbSetEchoFadeoutPos },
};

void drawScrollBar(uint16_t scrollBarID)
{
    int16_t thumbX, thumbY, thumbW, thumbH;
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];
    if (!scrollBar->visible)
        return;

    MY_ASSERT((scrollBar->x < SCREEN_W) && (scrollBar->y < SCREEN_H) && (scrollBar->w >= 3) && (scrollBar->h >= 3))

    thumbX = scrollBar->thumbX;
    thumbY = scrollBar->thumbY;
    thumbW = scrollBar->thumbW;
    thumbH = scrollBar->thumbH;

    /* clear scrollbar background */
    clearRect(scrollBar->x, scrollBar->y, scrollBar->w, scrollBar->h);

    /* draw thumb */
    if (scrollBar->thumbType == SCROLLBAR_THUMB_FLAT)
    {
        /* flat */
        if (scrollBar->state == SCROLLBAR_UNPRESSED)
            fillRect(thumbX, thumbY, thumbW, thumbH, PAL_PATTEXT);
        else
            fillRect(thumbX, thumbY, thumbW, thumbH, PAL_SCRLBAR);
    }
    else
    {
        /* 3D */
        fillRect(thumbX, thumbY, thumbW, thumbH, PAL_BUTTONS);

        if (scrollBar->state == SCROLLBAR_UNPRESSED)
        {
            /* top left corner inner border */
            hLine(thumbX, thumbY,     thumbW - 1, PAL_BUTTON1);
            vLine(thumbX, thumbY + 1, thumbH - 2, PAL_BUTTON1);

            /* bottom right corner inner border */
            hLine(thumbX,              thumbY + thumbH - 1, thumbW - 1, PAL_BUTTON2);
            vLine(thumbX + thumbW - 1, thumbY,              thumbH,     PAL_BUTTON2);
        }
        else
        {
            /* top left corner inner border */
            hLine(thumbX, thumbY,     thumbW,     PAL_BUTTON2);
            vLine(thumbX, thumbY + 1, thumbH - 1, PAL_BUTTON2);
        }
    }
}

void showScrollBar(uint16_t scrollBarID)
{
    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBars[scrollBarID].visible = true;
    drawScrollBar(scrollBarID);
}

void hideScrollBar(uint16_t scrollBarID)
{
    MY_ASSERT(scrollBarID < NUM_SCROLLBARS);

    scrollBars[scrollBarID].visible = false;
}

static void setScrollBarThumbCoords(uint16_t scrollBarID)
{
    int16_t thumbX, thumbY, thumbW, thumbH, scrollEnd;
    int32_t tmp32;
    double dTmp;
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];

    MY_ASSERT(scrollBar->page > 0)

    /* uninitialized scrollbar, set full thumb length/height */
    if (scrollBar->end == 0)
    {
        scrollBar->thumbX = scrollBar->x + 1;
        scrollBar->thumbY = scrollBar->y + 1;
        scrollBar->thumbW = scrollBar->w - 2;
        scrollBar->thumbH = scrollBar->h - 2;

        return;
    }

    if (scrollBar->type == SCROLLBAR_HORIZONTAL)
    {
        /* horizontal scrollbar */

        thumbY    = scrollBar->y + 1;
        thumbH    = scrollBar->h - 2;
        scrollEnd = scrollBar->x + scrollBar->w;

        if (scrollBar->thumbType == SCROLLBAR_THUMB_NOFLAT)
        {
            thumbW = 15;
            if (scrollBar->end > 0)
            {
                dTmp = ((scrollBar->w - thumbW) / (double)(scrollBar->end)) * (double)(scrollBar->pos);
                double2int32_round(tmp32, dTmp);
                thumbX = scrollBar->x + (int16_t)(tmp32);
            }
            else
            {
                thumbX = scrollBar->x;
            }
        }
        else
        {
            if (scrollBar->end > 0)
            {
                dTmp = (scrollBar->w / (double)(scrollBar->end)) * (double)(scrollBar->page);
                double2int32_round(tmp32, dTmp);
                thumbW = (int16_t)(CLAMP(tmp32, 1, scrollBar->w));
            }
            else
            {
                thumbW = 1;
            }

            if ((scrollBar->end - scrollBar->page) > 0)
            {
                dTmp = ((scrollBar->w - thumbW) / (double)(scrollBar->end - scrollBar->page)) * (double)(scrollBar->pos);
                double2int32_round(tmp32, dTmp);
                thumbX = scrollBar->x + (int16_t)(tmp32);
            }
            else
            {
                thumbX = scrollBar->x;
            }
        }

        /* prevent scrollbar thumb coords from being outside of the scrollbar area */
        thumbX = CLAMP(thumbX, scrollBar->x, scrollEnd - 1);
        if ((thumbX + thumbW) > scrollEnd)
            thumbW = scrollEnd - thumbX;
    }
    else
    {
        /* vertical scrollbar */

        thumbX    = scrollBar->x + 1;
        thumbW    = scrollBar->w - 2;
        scrollEnd = scrollBar->y + scrollBar->h;

        if (scrollBar->end > 0)
        {
            dTmp = (scrollBar->h / (double)(scrollBar->end)) * (double)(scrollBar->page);
            double2int32_round(tmp32, dTmp);
            thumbH = (int16_t)(CLAMP(tmp32, 1, scrollBar->h));
        }
        else
        {
            thumbH = 1;
        }

        if ((scrollBar->end - scrollBar->page) > 0)
        {
            dTmp = ((scrollBar->h - thumbH) / (double)(scrollBar->end - scrollBar->page)) * (double)(scrollBar->pos);
            double2int32_round(tmp32, dTmp);
            thumbY = scrollBar->y + (int16_t)(tmp32);
        }
        else
        {
            thumbY = scrollBar->y;
        }

        /* prevent scrollbar thumb coords from being outside of the scrollbar area */
        thumbY = CLAMP(thumbY, scrollBar->y, scrollEnd - 1);
        if ((thumbY + thumbH) > scrollEnd)
            thumbH = scrollEnd - thumbY;
    }

    /* set values now */
    scrollBar->thumbX = thumbX;
    scrollBar->thumbY = thumbY;
    scrollBar->thumbW = thumbW;
    scrollBar->thumbH = thumbH;
}

void scrollBarScrollUp(uint16_t scrollBarID, uint32_t amount)
{
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];

    MY_ASSERT((scrollBar->page > 0) && (scrollBar->end > 0))

    /* check if there is anything to scroll at all... */
    if (scrollBar->end < scrollBar->page)
        return;

    /* check if we're already at the beginning */
    if (scrollBar->pos == 0)
        return;

    scrollBar->pos -= amount;
    if (scrollBar->pos < 0)
        scrollBar->pos = 0;

    setScrollBarThumbCoords(scrollBarID);
    drawScrollBar(scrollBarID);

    if (scrollBar->callbackFunc != NULL)
        scrollBar->callbackFunc((int32_t)(scrollBar->pos));
}

void scrollBarScrollDown(uint16_t scrollBarID, uint32_t amount)
{
    int64_t endPos;
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];

    MY_ASSERT((scrollBar->page > 0) && (scrollBar->end > 0))

    /* check if there is anything to scroll at all... */
    if (scrollBar->end < scrollBar->page)
        return;

    endPos = scrollBar->end;
    if (scrollBar->thumbType == SCROLLBAR_THUMB_FLAT)
        endPos -= scrollBar->page;

    /* check if we're already at the end */
    if (scrollBar->pos == endPos)
        return;

    scrollBar->pos += amount;
    if (scrollBar->pos > endPos)
        scrollBar->pos = endPos;

    setScrollBarThumbCoords(scrollBarID);
    drawScrollBar(scrollBarID);

    if (scrollBar->callbackFunc != NULL)
        scrollBar->callbackFunc((int32_t)(scrollBar->pos));
}

void setScrollBarPos(uint16_t scrollBarID, int64_t pos, int8_t triggerCallBack)
{
    int64_t endPos;
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];

    if (scrollBar->page <= 0)
    {
        scrollBar->pos = 0;
        return;
    }

    if (pos < 0)
        pos = 0;

    /* check if there is anything to scroll at all... */
    if (scrollBar->end < scrollBar->page)
    {
        setScrollBarThumbCoords(scrollBarID);
        drawScrollBar(scrollBarID);
        return;
    }

    /* check if we're already at the same position */
    if (scrollBar->pos == pos)
    {
        setScrollBarThumbCoords(scrollBarID);
        drawScrollBar(scrollBarID);
        return;
    }

    endPos = scrollBar->end;
    if (scrollBar->thumbType == SCROLLBAR_THUMB_FLAT)
        endPos -= scrollBar->page;

    scrollBar->pos = pos;
    if (scrollBar->pos > endPos)
        scrollBar->pos = endPos;

    setScrollBarThumbCoords(scrollBarID);
    drawScrollBar(scrollBarID);

    if (triggerCallBack && (scrollBar->callbackFunc != NULL))
        scrollBar->callbackFunc((int32_t)(scrollBar->pos));
}

uint32_t getScrollBarPos(uint16_t scrollBarID)
{
    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    return ((uint32_t)(scrollBars[scrollBarID].pos));
}

void setScrollBarEnd(uint16_t scrollBarID, int64_t end)
{
    uint8_t setPos;
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    scrollBar = &scrollBars[scrollBarID];

    if (end < 1)
        end = 1;

    scrollBar->end = end;

    setPos = false;
    if (scrollBar->pos >= end)
    {
        scrollBar->pos  = end - 1;
        setPos = true;
    }

    if (scrollBar->page > 0)
    {
        if (setPos)
        {
            setScrollBarPos(scrollBarID, scrollBar->pos, false);
            /* this will also call setScrollBarThumbCoords() and drawScrollBar() */
        }
        else
        {
            setScrollBarThumbCoords(scrollBarID);
            drawScrollBar(scrollBarID);
        }
    }
}

void setScrollBarPageLength(uint16_t scrollBarID, int64_t pageLength)
{
    scrollBar_t *scrollBar;

    MY_ASSERT(scrollBarID < NUM_SCROLLBARS)

    if (pageLength < 1)
        pageLength = 1;

    scrollBar = &scrollBars[scrollBarID];

    scrollBar->page = pageLength;
    if (scrollBar->end > 0)
    {
        setScrollBarPos(scrollBarID, scrollBar->pos, false);
        setScrollBarThumbCoords(scrollBarID);
        drawScrollBar(scrollBarID);
    }
}

int8_t testScrollBarMouseDown(void)
{
    uint16_t i;
    int32_t scrollPos;
    double dTmp;
    scrollBar_t *scrollBar;

    for (i = 0; i < NUM_SCROLLBARS; ++i)
    {
        scrollBar = &scrollBars[i];
        if (scrollBar->visible)
        {
            /* if sys req. is shown, only allow certain scrollbars */
            if (editor.ui.systemRequestShown)
            {
                switch (i)
                {
                    default: continue;

                    case SB_SAMPVOL_START:
                    case SB_SAMPVOL_END:
                    case SB_RESAMPLE_HTONES:
                    case SB_MIX_BALANCE:
                    case SB_ECHO_NUM:
                    case SB_ECHO_DISTANCE:
                    case SB_ECHO_FADEOUT:
                        break;
                }
            }

            if ((mouse.x >= scrollBar->x) && (mouse.x < (scrollBar->x + scrollBar->w)))
            {
                if ((mouse.y >= scrollBar->y) && (mouse.y < (scrollBar->y + scrollBar->h)))
                {
                    mouse.lastUsedObjectID   = i;
                    mouse.lastUsedObjectType = OBJECT_SCROLLBAR;

                    if (scrollBar->type == SCROLLBAR_HORIZONTAL)
                    {
                        mouse.lastScrollXTmp = mouse.lastScrollX = mouse.x;

                        if ((mouse.x >= scrollBar->thumbX) && (mouse.x < (scrollBar->thumbX + scrollBar->thumbW)))
                        {
                            mouse.saveMouseX = mouse.lastScrollX - scrollBar->thumbX;
                        }
                        else
                        {
                            dTmp = scrollBar->thumbW / 2.0;
                            double2int32_round(mouse.saveMouseX, dTmp);

                            scrollPos = mouse.lastScrollX - scrollBar->x - mouse.saveMouseX;
                            if (scrollBar->thumbType == SCROLLBAR_THUMB_NOFLAT)
                            {
                                dTmp = scrollPos * (scrollBar->w / (scrollBar->w - 15.0));
                                double2int32_round(scrollPos, dTmp);
                            }

                            scrollPos = CLAMP(scrollPos, 0, scrollBar->w);

                            MY_ASSERT(scrollBar->w > 0)

                            dTmp = ((scrollPos * scrollBar->end) / (double)(scrollBar->w)) + 0.5;
                            setScrollBarPos(mouse.lastUsedObjectID, (int64_t)(dTmp), true);
                        }
                    }
                    else
                    {
                        mouse.lastScrollY = mouse.y;
                        if ((mouse.y >= scrollBar->thumbY) && (mouse.y < (scrollBar->thumbY + scrollBar->thumbH)))
                        {
                            mouse.saveMouseY = mouse.lastScrollY - scrollBar->thumbY;
                        }
                        else
                        {
                            mouse.saveMouseY = (int32_t)(scrollBar->thumbH / 2.0); /* truncate here */

                            scrollPos = mouse.lastScrollY - scrollBar->y - mouse.saveMouseY;
                            scrollPos = CLAMP(scrollPos, 0, scrollBar->h);

                            MY_ASSERT(scrollBar->h > 0)

                            dTmp = ((scrollPos * scrollBar->end) / (double)(scrollBar->h)) + 0.5;
                            setScrollBarPos(mouse.lastUsedObjectID, (int64_t)(dTmp), true);
                        }
                    }

                    scrollBar->state = SCROLLBAR_PRESSED;
                    drawScrollBar(mouse.lastUsedObjectID);

                    return (true);
                }
            }
        }
    }

    return (false);
}

void testScrollBarMouseRelease(void)
{
    scrollBar_t *scrollBar;

    if (mouse.lastUsedObjectType == OBJECT_SCROLLBAR)
    {
        if (mouse.lastUsedObjectID != OBJECT_ID_NONE)
        {
            MY_ASSERT(mouse.lastUsedObjectID < NUM_SCROLLBARS)

            scrollBar = &scrollBars[mouse.lastUsedObjectID];
            if (scrollBar->visible)
            {
                scrollBar->state = SCROLLBAR_UNPRESSED;
                drawScrollBar(mouse.lastUsedObjectID);
            }
        }
    }
}

void handleScrollBarsWhileMouseDown(void)
{
    int32_t scrollX, scrollY;
    double dTmp;
    scrollBar_t *scrollBar;

    MY_ASSERT((mouse.lastUsedObjectID >= 0) && (mouse.lastUsedObjectID < NUM_SCROLLBARS))

    scrollBar = &scrollBars[mouse.lastUsedObjectID];
    if (!scrollBar->visible)
        return;

    if (scrollBar->type == SCROLLBAR_HORIZONTAL)
    {
        if (mouse.x != mouse.lastScrollX)
        {
            mouse.lastScrollX = mouse.x;
            scrollX = mouse.lastScrollX - mouse.saveMouseX - scrollBar->x;

            if (scrollBar->thumbType == SCROLLBAR_THUMB_NOFLAT)
            {
                MY_ASSERT(scrollBar->w >= 16)

                dTmp = scrollX * (scrollBar->w / (double)(scrollBar->w - 15));
                double2int32_round(scrollX, dTmp);
            }

            scrollX = CLAMP(scrollX, 0, scrollBar->w);

            MY_ASSERT(scrollBar->w > 0)

            dTmp = ((scrollX * scrollBar->end) / (double)(scrollBar->w)) + 0.5;
            setScrollBarPos(mouse.lastUsedObjectID, (int64_t)(dTmp), true);
            drawScrollBar(mouse.lastUsedObjectID);
        }
    }
    else
    {
        if (mouse.y != mouse.lastScrollY)
        {
            mouse.lastScrollY = mouse.y;

            scrollY = mouse.lastScrollY - mouse.saveMouseY - scrollBar->y;
            scrollY = CLAMP(scrollY, 0, scrollBar->h);

            MY_ASSERT(scrollBar->h > 0)

            dTmp = ((scrollY * scrollBar->end) / (double)(scrollBar->h)) + 0.5;
            setScrollBarPos(mouse.lastUsedObjectID, (int64_t)(dTmp), true);
            drawScrollBar(mouse.lastUsedObjectID);
        }
    }
}

/* used to create a darker color used when holding the scrollbar thumb down */
void updateScrollBarPalette(void)
{
    uint32_t pal;
    int16_t r, g, b;

    pal = video.palette[PAL_PATTEXT];

    r = RGB_R(pal);
    g = RGB_G(pal);
    b = RGB_B(pal);

    r -= 54;
    g -= 54;
    b -= 54;

    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;

    video.palette[PAL_SCRLBAR] = TO_RGB(r, g, b);
}

void initializeScrollBars(void)
{
    /* pattern editor */
    setScrollBarPageLength(SB_CHAN_SCROLL, 8);
    setScrollBarEnd(SB_CHAN_SCROLL, 8);

    /* position editor */
    setScrollBarPageLength(SB_POS_ED, 5);
    setScrollBarEnd(SB_POS_ED, 5);

    /* instrument switcher */
    setScrollBarPageLength(SB_SAMPLE_LIST, 5);
    setScrollBarEnd(SB_SAMPLE_LIST, 16);

    /* help screen */
    setScrollBarPageLength(SB_HELP_SCROLL, HELP_WINDOW_LINES);
    setScrollBarEnd(SB_HELP_SCROLL, 1);

    /* config screen */
    setScrollBarPageLength(SB_AMP_SCROLL, 1);
    setScrollBarEnd(SB_AMP_SCROLL, 31);
    setScrollBarPageLength(SB_MASTERVOL_SCROLL, 1);
    setScrollBarEnd(SB_MASTERVOL_SCROLL, 256);
    setScrollBarPageLength(SB_PAL_R, 1);
    setScrollBarEnd(SB_PAL_R, 63);
    setScrollBarPageLength(SB_PAL_G, 1);
    setScrollBarEnd(SB_PAL_G, 63);
    setScrollBarPageLength(SB_PAL_B, 1);
    setScrollBarEnd(SB_PAL_B, 63);
    setScrollBarPageLength(SB_PAL_CONTRAST, 1);
    setScrollBarEnd(SB_PAL_CONTRAST, 100);
    setScrollBarPageLength(SB_MIDI_SENS, 1);
    setScrollBarEnd(SB_MIDI_SENS, 200);
    setScrollBarPageLength(SB_AUDIO_OUTPUT_SCROLL, 6);
    setScrollBarEnd(SB_AUDIO_OUTPUT_SCROLL, 1);
    setScrollBarPageLength(SB_AUDIO_INPUT_SCROLL, 6);
    setScrollBarEnd(SB_AUDIO_INPUT_SCROLL, 1);
    setScrollBarPageLength(SB_MIDI_INPUT_SCROLL, 15);
    setScrollBarEnd(SB_MIDI_INPUT_SCROLL, 1);

    /* disk op. */
    setScrollBarPageLength(SB_DISKOP_LIST, DISKOP_ENTRY_NUM);
    setScrollBarEnd(SB_DISKOP_LIST, 1);

    /* instrument editor */
    setScrollBarPageLength(SB_INST_VOL, 1);
    setScrollBarEnd(SB_INST_VOL, 64);
    setScrollBarPageLength(SB_INST_PAN, 1);
    setScrollBarEnd(SB_INST_PAN, 255);
    setScrollBarPageLength(SB_INST_FTUNE, 1);
    setScrollBarEnd(SB_INST_FTUNE, 255);
    setScrollBarPageLength(SB_INST_FADEOUT, 1);
    setScrollBarEnd(SB_INST_FADEOUT, 0xFFF);
    setScrollBarPageLength(SB_INST_VIBSPEED, 1);
    setScrollBarEnd(SB_INST_VIBSPEED, 0x3F);
    setScrollBarPageLength(SB_INST_VIBDEPTH, 1);
    setScrollBarEnd(SB_INST_VIBDEPTH, 0xF);
    setScrollBarPageLength(SB_INST_VIBSWEEP, 1);
    setScrollBarEnd(SB_INST_VIBSWEEP, 0xFF);

    /* instrument editor extension */
    setScrollBarPageLength(SB_INST_EXT_MIDI_CH, 1);
    setScrollBarEnd(SB_INST_EXT_MIDI_CH, 15);
    setScrollBarPageLength(SB_INST_EXT_MIDI_PRG, 1);
    setScrollBarEnd(SB_INST_EXT_MIDI_PRG, 127);
    setScrollBarPageLength(SB_INST_EXT_MIDI_BEND, 1);
    setScrollBarEnd(SB_INST_EXT_MIDI_BEND, 36);

    /* sample volume box */
    setScrollBarPageLength(SB_SAMPVOL_START, 1);
    setScrollBarEnd(SB_SAMPVOL_START, 400 * 2);
    setScrollBarPos(SB_SAMPVOL_START, 400, false);
    setScrollBarPageLength(SB_SAMPVOL_END, 1);
    setScrollBarEnd(SB_SAMPVOL_END, 400 * 2);
    setScrollBarPos(SB_SAMPVOL_END, 400, false);

    /* sample resample box */
    setScrollBarPageLength(SB_RESAMPLE_HTONES, 1);
    setScrollBarEnd(SB_RESAMPLE_HTONES, 36 * 2);

    /* sample mix sample box */
    setScrollBarPageLength(SB_MIX_BALANCE, 1);
    setScrollBarEnd(SB_MIX_BALANCE, 100);

    /* sample echo vox */
    setScrollBarPageLength(SB_ECHO_NUM, 1);
    setScrollBarEnd(SB_ECHO_NUM, 1024);
    setScrollBarPageLength(SB_ECHO_DISTANCE, 1);
    setScrollBarEnd(SB_ECHO_DISTANCE, 16384);
    setScrollBarPageLength(SB_ECHO_FADEOUT, 1);
    setScrollBarEnd(SB_ECHO_FADEOUT, 100);
}
