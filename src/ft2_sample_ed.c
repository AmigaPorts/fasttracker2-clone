/* for finding memory leaks in debug mode with Visual Studio */
#if defined _DEBUG && defined _MSC_VER
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#ifndef _WIN32
#include <unistd.h> /* chdir() in UNICHAR_CHDIR() */
#endif
#include "ft2_header.h"
#include "ft2_config.h"
#include "ft2_audio.h"
#include "ft2_pattern_ed.h"
#include "ft2_gui.h"
#include "ft2_scopes.h"
#include "ft2_video.h"
#include "ft2_inst_ed.h"
#include "ft2_sample_ed.h"
#include "ft2_sample_saver.h"
#include "ft2_mouse.h"
#include "ft2_diskop.h"
#include "ft2_keyboard.h"

static char smpEd_SysReqText[128];
static int8_t *smpCopyBuff;
static uint8_t updateLoopsOnMouseUp;
static int32_t smpEd_OldSmpPosLine = -1; /* must be initialized to -1! */
static int32_t smpEd_ViewSize, smpEd_ScrPos, smpCopySize, smpCopyBits;
static int32_t old_Rx1, old_Rx2, old_ViewSize, old_SmpScrPos;
static int32_t lastMouseX, lastMouseY, lastDrawX, lastDrawY, mouseXOffs, curSmpRepS, curSmpRepL;

/* globals */
int32_t smpEd_Rx1 = 0, smpEd_Rx2 = 0;
sampleTyp *currSmp = NULL;

/* adds wrapped sample after loop/end (for branchless linear interpolation) */
void fixSample(sampleTyp *s)
{
    uint8_t loopType;
    int16_t *ptr16;
    int32_t loopStart, loopEnd, len;

    len = s->len;
    if (s->pek == NULL)
        return; /* empty sample */

    loopType = s->typ & 3;
    if (loopType == 0)
    {
        /* no loop (don't mess with fixed, fixSpar of fixedPos) */

        if (s->typ & 16)
        {
            if (len < 2)
                return;

            len  /= 2;
            ptr16 = (int16_t *)(s->pek);

            ptr16[len] = 0;
        }
        else
        {
            if (len < 1)
                return;

            s->pek[len] = 0;
        }

        return;
    }

    if (s->fixed)
        return; /* already fixed */

    if (loopType == 1)
    {
        /* forward loop */

        if (s->typ & 16)
        {
            /* 16-bit sample */

            if (s->repL < 2)
                return;

            loopStart = s->repS / 2;
            loopEnd   = (s->repS + s->repL) / 2;
            ptr16     = (int16_t *)(s->pek);

            s->fixSpar = ptr16[loopEnd]; /* store old value */
            ptr16[loopEnd] = ptr16[loopStart]; /* write new value */

            s->fixedPos = s->repS + s->repL;
            s->fixed = true;
        }
        else
        {
            /* 8-bit sample */

            if (s->repL < 1)
                return;

            loopEnd = s->repS + s->repL;

            s->fixSpar = s->pek[loopEnd]; /* store old value */
            s->pek[loopEnd] = s->pek[s->repS]; /* write new value */

            s->fixedPos = loopEnd;
            s->fixed = true;
        }
    }
    else
    {
        /* bidi loop */

        if (s->typ & 16)
        {
            /* 16-bit sample */

            if (s->repL < 2)
                return;

            loopEnd = (s->repS + s->repL) / 2;
            ptr16   = (int16_t *)(s->pek);

            s->fixSpar = ptr16[loopEnd]; /* store old value */
            ptr16[loopEnd] = ptr16[loopEnd - 1];

            s->fixedPos = s->repS + s->repL;
            s->fixed    = true;
        }
        else
        {
            /* 8-bit sample */

            if (s->repL < 1)
                return;

            loopEnd = s->repS + s->repL;

            s->fixSpar = s->pek[loopEnd]; /* store old value */
            s->pek[loopEnd] = s->pek[loopEnd - 1];

            s->fixedPos = loopEnd;
            s->fixed    = true;
        }
    }
}

/* reverts wrapped sample after loop/end (for branchless linear interpolation) */
void restoreSample(sampleTyp *s)
{
    int16_t *ptr16;

    if ((s->pek == NULL) || !(s->typ & 3) || !s->fixed)
        return; /* empty sample, no loop or not fixed */

    s->fixed = false;

    if (s->typ & 16)
    {
        /* 16-bit sample */

        MY_ASSERT((s->len >= 4) && (s->fixedPos < (s->len + 2)) && !(s->fixedPos & 1))

        ptr16 = (int16_t *)(s->pek);
        ptr16[s->fixedPos / 2] = s->fixSpar;
    }
    else
    {
        /* 8-bit sample */

        MY_ASSERT((s->len >= 2) && (s->fixedPos < (s->len + 2)))

        s->pek[s->fixedPos] = (int8_t)(s->fixSpar);
    }
}

int16_t getSampleValueNr(int8_t *ptr, uint8_t typ, int32_t pos)
{
    if ((ptr == NULL) || (pos < 0))
        return (0);

    if (typ & 16)
    {
        MY_ASSERT(!(pos & 1))
        return (*((int16_t *)(&ptr[pos])));
    }
    else
    {
        return ((int16_t)(ptr[pos]));
    }
}

void putSampleValueNr(int8_t *ptr, uint8_t typ, int32_t pos, int16_t val)
{
    if ((ptr == NULL) || (pos < 0))
        return;

    if (typ & 16)
    {
        MY_ASSERT(!(pos & 1))
        *((int16_t *)(&ptr[pos])) = val;
    }
    else
    {
        ptr[pos] = (int8_t)(val);
    }
}

void clearCopyBuffer(void)
{
    if (smpCopyBuff != NULL)
    {
        free(smpCopyBuff);
        smpCopyBuff = NULL;
    }

    smpCopySize = 0;
    smpCopyBits = 8;
}

uint32_t getSampleMiddleCRate(sampleTyp *s)
{
    double dFtune;

    /* FT2 fix: get correct finetune value (replayer is shifting it to the right by 3) */
    dFtune = (s->fine >> 3) / (128.0 / (double)(1 << 3));

    return ((uint32_t)(round(8363.0 * pow(2.0, (s->relTon + dFtune) / 12.0))));
}

int32_t getSampleRangeStart(void)
{
    return (smpEd_Rx1);
}

int32_t getSampleRangeEnd(void)
{
    return (smpEd_Rx2);
}

int32_t getSampleRangeLength(void)
{
    return (smpEd_Rx2 - smpEd_Rx1);
}

static int32_t smpPos2Scr(int32_t pos) /* sample pos -> screen x pos (result can and will overflow) */
{
    uint8_t roundFlag;
    int32_t scaledPos;
    double dScaledPos, dScaledOffset;

    MY_ASSERT(currSmp != NULL)

    if (smpEd_ViewSize <= 0)
        return (0);

    if (pos > currSmp->len)
        pos = currSmp->len;

    roundFlag = smpEd_ViewSize >= SAMPLE_AREA_WIDTH;

    dScaledPos = (pos / (double)(smpEd_ViewSize)) * SAMPLE_AREA_WIDTH;
    if (roundFlag)
        dScaledPos = round(dScaledPos);

    dScaledOffset = (smpEd_ScrPos / (double)(smpEd_ViewSize)) * SAMPLE_AREA_WIDTH;
    if (roundFlag)
        dScaledOffset = round(dScaledOffset);

    dScaledPos -= dScaledOffset;
    if (roundFlag)
        double2int32_round(scaledPos, dScaledPos);
    else
        scaledPos = (int32_t)(dScaledPos);

    return (scaledPos);
}

int32_t scr2SmpPos(int32_t x) /* screen x pos -> sample pos */
{
    uint8_t roundFlag;
    int32_t scaledPos;
    double dScaledPos;

    MY_ASSERT(currSmp != NULL)

    if (smpEd_ViewSize <= 0)
        return (0);

    if (x < 0)
        x = 0;

    roundFlag = smpEd_ViewSize >= SAMPLE_AREA_WIDTH;

    dScaledPos = (smpEd_ScrPos / (double)(smpEd_ViewSize)) * SAMPLE_AREA_WIDTH;
    if (roundFlag)
        dScaledPos = round(dScaledPos);
    else
        dScaledPos = floor(dScaledPos);

    dScaledPos += x;

    dScaledPos = (dScaledPos / SAMPLE_AREA_WIDTH) * smpEd_ViewSize;
    if (roundFlag)
        double2int32_round(scaledPos, dScaledPos);
    else
        scaledPos = (int32_t)(dScaledPos); /* truncate */

    if (scaledPos > currSmp->len)
        scaledPos = currSmp->len;

    if (currSmp->typ & 16)
        scaledPos &= 0xFFFFFFFE; /* align to 16-bit because of 8-bit offset look-ups */

    return (scaledPos);
}

void fixRepeatGadgets(void)
{
    int32_t repS, repE;

    if ((currSmp->len <= 0) || (currSmp->pek == NULL) || ((currSmp->typ & 3) == 0))
    {
        hideSprite(SPRITE_LEFT_LOOP_PIN);
        hideSprite(SPRITE_RIGHT_LOOP_PIN);
    }
    else
    {
        /* draw sample loop points */

        repS = smpPos2Scr(curSmpRepS);
        repE = smpPos2Scr(curSmpRepS + curSmpRepL);

        /* do -8 test because part of the loop sprite sticks out on the left/right */

        if ((repS >= -8) && (repS <= (SAMPLE_AREA_WIDTH + 8)))
            setSpritePos(SPRITE_LEFT_LOOP_PIN, (int16_t)(repS - 8), 174);
        else
            hideSprite(SPRITE_LEFT_LOOP_PIN);

        if (repE >= -8)
        {
            if (repE <= (SAMPLE_AREA_WIDTH + 8))
                setSpritePos(SPRITE_RIGHT_LOOP_PIN, (int16_t)(repE - 8), 174);
            else
                hideSprite(SPRITE_RIGHT_LOOP_PIN);
        }
        else
        {
            hideSprite(SPRITE_RIGHT_LOOP_PIN);
        }
    }

    if (editor.ui.sampleEditorShown)
    {
        fillRect(536, 375, 56, 20, PAL_DESKTOP);
        hexOut(536, 375, PAL_FORGRND, curSmpRepS, 8);
        hexOut(536, 387, PAL_FORGRND, curSmpRepL, 8);
    }
}

void fixSampleDrag(void)
{
    setScrollBarPageLength(SB_SAMP_SCROLL, smpEd_ViewSize);
    setScrollBarEnd(SB_SAMP_SCROLL, currSmp->len);
    setScrollBarPos(SB_SAMP_SCROLL, smpEd_ScrPos, false);
}

int8_t getCopyBuffer(int32_t size)
{
    if (smpCopyBuff != NULL)
        free(smpCopyBuff);

    smpCopyBuff = (int8_t *)(malloc(size));
    if (smpCopyBuff == NULL)
    {
        smpCopySize = 0;
        return (false);
    }

    smpCopySize = size;
    return (true);
}

void copySmp(void) /* copy sample from srcInstr->srcSmp to curInstr->curSmp */
{
    int8_t *p;
    sampleTyp *src, *dst;

    if ((editor.curInstr == 0) || ((editor.curInstr == editor.srcInstr) && (editor.curSmp == editor.srcSmp)))
        return;

    src = &instr[editor.srcInstr].samp[editor.srcSmp];
    dst = &instr[editor.curInstr].samp[editor.curSmp];

    pauseAudio();

    if (src->pek != NULL)
    {
        p = (int8_t *)(malloc(src->len + 2));
        if (p != NULL)
        {
            memcpy(dst, src, sizeof (sampleTyp));
            memcpy(p, src->pek, src->len + 2); /* +2 = include loop unroll area */
            dst->pek = p;
        }
        else
        {
            okBox(0, "System message", "Not enough memory!");
        }
    }

    resumeAudio();

    updateNewSample();
    setSongModifiedFlag();
}

void xchgSmp(void) /* dstSmp <-> srcSmp */
{
    sampleTyp *src, *dst, dstTmp;

    if ((editor.curInstr == 0) || ((editor.curInstr == editor.srcInstr) && (editor.curSmp == editor.srcSmp)))
        return;

    src = &instr[editor.curInstr].samp[editor.srcSmp];
    dst = currSmp;

    lockMixerCallback();
    dstTmp = *dst;
    *dst   = *src;
    *src   = dstTmp;
    unlockMixerCallback();

    updateNewSample();
    setSongModifiedFlag();
}

void writeRange(void)
{
    int32_t x, y, start, end, rangeLen;
    uint32_t *ptr32;

    /* very first sample (rx1=0,rx2=0) is the "no range" special case */
    if (!editor.ui.sampleEditorShown || (smpEd_ViewSize == 0) || ((smpEd_Rx1 == 0) && (smpEd_Rx2 == 0)))
        return; 

    start = smpPos2Scr(smpEd_Rx1);
    end   = smpPos2Scr(smpEd_Rx2);

    /* test if range is outside of view (passed it by scrolling) */
    if ((start >= SAMPLE_AREA_WIDTH) || (end < 0))
        return;

    start    = CLAMP(start, 0, SAMPLE_AREA_WIDTH - 1);
    end      = CLAMP(end,   0, SAMPLE_AREA_WIDTH - 1);
    rangeLen = (end + 1) - start;

    MY_ASSERT((start + rangeLen) <= SCREEN_W)

    ptr32 = &video.frameBuffer[(174 * SCREEN_W) + start];
    for (y = 0; y < SAMPLE_AREA_HEIGHT; ++y)
    {
        for (x = 0; x < rangeLen; ++x)
            ptr32[x] = video.palette[(ptr32[x] >> 24) ^ 2]; /* ">> 24" to get palette from pixel, XOR 2 to switch between mark/normal palette */

        ptr32 += SCREEN_W;
    }
}

int8_t getScaledSample(int32_t index)
{
    int8_t *ptr8, sample;
    int16_t *ptr16;
    int32_t tmp32;

    if ((index < 0) || (index >= currSmp->len))
        return (0); /* return center value if overflown (e.g. sample is shorter than screen width) */

    if (currSmp->typ & 16)
    {
        ptr16 = (int16_t *)(currSmp->pek);
        if (ptr16 == NULL)
            return (0);

        MY_ASSERT(!(index & 1))

        /* restore fixed linear interpolation sample after loop end */
        if (currSmp->fixed && (index == currSmp->fixedPos))
            tmp32 = currSmp->fixSpar * SAMPLE_AREA_HEIGHT;
        else
            tmp32 = ptr16[index / 2] * SAMPLE_AREA_HEIGHT;

        sample = (int8_t)(tmp32 >> 16);
    }
    else
    {
        ptr8 = currSmp->pek;
        if (ptr8 == NULL)
            return (0);

        /* restore fixed linear interpolation sample after loop end */
        if (currSmp->fixed && (index == currSmp->fixedPos))
            tmp32 = (int8_t)(currSmp->fixSpar) * SAMPLE_AREA_HEIGHT;
        else
            tmp32 = ptr8[index] * SAMPLE_AREA_HEIGHT;

        sample = (int8_t)(tmp32 >> 8);
    }

    MY_ASSERT((sample >= -77) && (sample <= 76))

    return (sample);
}

void sampleLine(int16_t x1, int16_t x2, int16_t y1, int16_t y2)
{
    int16_t d, x, y, sx, sy, dx, dy;
    uint16_t ax, ay;
    int32_t pitch;
    uint32_t pal1, pal2, pixVal, *dst32;

    /* get coefficients */
    dx = x2 - x1;
    ax = ABS(dx) * 2;
    sx = SGN(dx);
    dy = y2 - y1;
    ay = ABS(dy) * 2;
    sy = SGN(dy);
    x  = x1;
    y  = y1;

    pal1   = video.palette[PAL_DESKTOP];
    pal2   = video.palette[PAL_FORGRND];
    pixVal = video.palette[PAL_PATTEXT];
    pitch  = sy * SCREEN_W;

    dst32 = &video.frameBuffer[(y * SCREEN_W) + x];

    /* draw line */
    if (ax > ay)
    {
        d = ay - (ax / 2);

        while (true)
        {
            /* invert certain colors */
            if (*dst32 != pal2)
            {
                if (*dst32 == pal1)
                    *dst32 = pal2;
                else
                    *dst32 = pixVal;
            }

            if (x == x2)
                break;

            if (d >= 0)
            {
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
            /* invert certain colors */
            if (*dst32 != pal2)
            {
                if (*dst32 == pal1)
                    *dst32 = pal2;
                else
                    *dst32 = pixVal;
            }

            if (y == y2)
                break;

            if (d >= 0)
            {
                d -= ay;
                dst32 += sx;
            }

            y += sy;
            d += ax;
            dst32 += pitch;
        }
    }
}

static void getMinMax16(const void *p, uint32_t scanLen, int16_t *min16, int16_t *max16)
{
#if defined __APPLE__ || defined _WIN32 || defined __i386__ || defined __amd64__
    /* Taken with permission from the OpenMPT project (and slightly modified).
    **
    ** SSE2 implementation for min/max finder, packs 8*int16 in a 128-bit XMM register.
    ** scanLen = How many samples to process
    */
    const int16_t *p16;
    uint32_t scanLen8;
    const __m128i *v;
    __m128i minVal, maxVal, minVal2, maxVal2, curVals;

    /* Put minimum / maximum in 8 packed int16 values */
    minVal = _mm_set1_epi16(32767);
    maxVal = _mm_set1_epi16(-32768);

    scanLen8 = scanLen / 8;
    if (scanLen8 > 0)
    {
        v = (__m128i *)(p);
        p = (const __m128i *)(p) + scanLen8;

        while (scanLen8--)
        {
            curVals = _mm_loadu_si128(v++);
            minVal  = _mm_min_epi16(minVal, curVals);
            maxVal  = _mm_max_epi16(maxVal, curVals);
        }

        /* Now we have 8 minima and maxima each.
        ** Move the upper 4 values to the lower half and compute the minima/maxima of that. */
        minVal2 = _mm_unpackhi_epi64(minVal, minVal);
        maxVal2 = _mm_unpackhi_epi64(maxVal, maxVal);
        minVal  = _mm_min_epi16(minVal, minVal2);
        maxVal  = _mm_max_epi16(maxVal, maxVal2);

        /* Now we have 4 minima and maxima each.
        ** Move the upper 2 values to the lower half and compute the minima/maxima of that. */
        minVal2 = _mm_shuffle_epi32(minVal, _MM_SHUFFLE(1, 1, 1, 1));
        maxVal2 = _mm_shuffle_epi32(maxVal, _MM_SHUFFLE(1, 1, 1, 1));
        minVal  = _mm_min_epi16(minVal, minVal2);
        maxVal  = _mm_max_epi16(maxVal, maxVal2);

        /* Compute the minima/maxima of the both remaining values */
        minVal2 = _mm_shufflelo_epi16(minVal, _MM_SHUFFLE(1, 1, 1, 1));
        maxVal2 = _mm_shufflelo_epi16(maxVal, _MM_SHUFFLE(1, 1, 1, 1));
        minVal  = _mm_min_epi16(minVal, minVal2);
        maxVal  = _mm_max_epi16(maxVal, maxVal2);
    }

    p16 = (const int16_t *)(p);
    while (scanLen-- & 7)
    {
        curVals = _mm_set1_epi16(*p16++);
        minVal  = _mm_min_epi16(minVal, curVals);
        maxVal  = _mm_max_epi16(maxVal, curVals);
    }

    *min16 = (int16_t)(_mm_cvtsi128_si32(minVal));
    *max16 = (int16_t)(_mm_cvtsi128_si32(maxVal));
#else
    /* non-SSE version (slow!) */
    int16_t smp16, minVal, maxVal, *ptr16;
    uint32_t i;

    minVal =  32767;
    maxVal = -32768;

    ptr16 = (int16_t *)(p);
    for (i = 0; i < scanLen; ++i)
    {
        smp16 = ptr16[i];
        if (smp16 < minVal) minVal = smp16;
        if (smp16 > maxVal) maxVal = smp16;
    }

    *min16 = minVal;
    *max16 = maxVal;
#endif
}

static void getMinMax8(const void *p, uint32_t scanLen, int8_t *min8, int8_t *max8)
{
#if defined __APPLE__ || defined _WIN32 || defined __i386__ || defined __amd64__
    /* Taken with permission from the OpenMPT project (and slightly modified).
    **
    ** SSE2 implementation for min/max finder, packs 16*int8 in a 128-bit XMM register.
    ** scanLen = How many samples to process
    */
    const int8_t *p8;
    uint32_t scanLen16;
    const __m128i *v;
    __m128i xorVal, minVal, maxVal, minVal2, maxVal2, curVals;

    /* Put minimum / maximum in 8 packed int16 values (-1 and 0 because unsigned) */
    minVal = _mm_set1_epi8(-1);
    maxVal = _mm_set1_epi8(0);

    /* For signed <-> unsigned conversion (_mm_min_epi8/_mm_max_epi8 is SSE4) */
    xorVal = _mm_set1_epi8(0x80);

    scanLen16 = scanLen / 16;
    if (scanLen16 > 0)
    {
        v = (__m128i *)(p);
        p = (const __m128i *)(p) + scanLen16;

        while (scanLen16--)
        {
            curVals = _mm_loadu_si128(v++);
            curVals = _mm_xor_si128(curVals, xorVal);
            minVal  = _mm_min_epu8(minVal, curVals);
            maxVal  = _mm_max_epu8(maxVal, curVals);
        }

        /* Now we have 16 minima and maxima each.
        ** Move the upper 8 values to the lower half and compute the minima/maxima of that. */
        minVal2 = _mm_unpackhi_epi64(minVal, minVal);
        maxVal2 = _mm_unpackhi_epi64(maxVal, maxVal);
        minVal  = _mm_min_epu8(minVal, minVal2);
        maxVal  = _mm_max_epu8(maxVal, maxVal2);

        /* Now we have 8 minima and maxima each.
        ** Move the upper 4 values to the lower half and compute the minima/maxima of that. */
        minVal2 = _mm_shuffle_epi32(minVal, _MM_SHUFFLE(1, 1, 1, 1));
        maxVal2 = _mm_shuffle_epi32(maxVal, _MM_SHUFFLE(1, 1, 1, 1));
        minVal  = _mm_min_epu8(minVal, minVal2);
        maxVal  = _mm_max_epu8(maxVal, maxVal2);

        /* Now we have 4 minima and maxima each.
        ** Move the upper 2 values to the lower half and compute the minima/maxima of that. */
        minVal2 = _mm_srai_epi32(minVal, 16);
        maxVal2 = _mm_srai_epi32(maxVal, 16);
        minVal  = _mm_min_epu8(minVal, minVal2);
        maxVal  = _mm_max_epu8(maxVal, maxVal2);

        /* Compute the minima/maxima of the both remaining values */
        minVal2 = _mm_srai_epi16(minVal, 8);
        maxVal2 = _mm_srai_epi16(maxVal, 8);
        minVal  = _mm_min_epu8(minVal, minVal2);
        maxVal  = _mm_max_epu8(maxVal, maxVal2);
    }

    p8 = (const int8_t *)(p);
    while (scanLen-- & 15)
    {
        curVals = _mm_set1_epi8((*p8++) ^ 0x80);
        minVal  = _mm_min_epu8(minVal, curVals);
        maxVal  = _mm_max_epu8(maxVal, curVals);
    }

    *min8 = (int8_t)(_mm_cvtsi128_si32(minVal) ^ 0x80);
    *max8 = (int8_t)(_mm_cvtsi128_si32(maxVal) ^ 0x80);
#else
    /* non-SSE version (slow!) */
    int8_t smp8, minVal, maxVal, *ptr8;
    uint32_t i;

    minVal =  127;
    maxVal = -128;

    ptr8 = (int8_t *)(p);
    for (i = 0; i < scanLen; ++i)
    {
        smp8 = ptr8[i];
        if (smp8 < minVal) minVal = smp8;
        if (smp8 > maxVal) maxVal = smp8;
    }

    *min8 = minVal;
    *max8 = maxVal;
#endif
}

void getSampleDataPeak(int32_t index, int32_t numBytes, int16_t *outMin, int16_t *outMax)
{
    int8_t min8, max8;
    int16_t min16, max16;

    /* prevent look-up overflow (yes, this can happen near the end of the sample) */
    if ((index + numBytes) > currSmp->len)
        numBytes = currSmp->len - index;

    if ((currSmp->pek == NULL) || (numBytes == 0))
    {
        *outMin = SAMPLE_AREA_Y_CENTER;
        *outMax = SAMPLE_AREA_Y_CENTER;
        return;
    }

    if (currSmp->typ & 16)
    {
        /* 16-bit sample */

        MY_ASSERT(!(index & 1))

        getMinMax16((int16_t *)(&currSmp->pek[index]), numBytes / 2, &min16, &max16);

        *outMin = SAMPLE_AREA_Y_CENTER - ((min16 * SAMPLE_AREA_HEIGHT) >> 16);
        *outMax = SAMPLE_AREA_Y_CENTER - ((max16 * SAMPLE_AREA_HEIGHT) >> 16);
    }
    else
    {
        /* 8-bit sample */

        getMinMax8(&currSmp->pek[index], numBytes, &min8, &max8);

        *outMin = SAMPLE_AREA_Y_CENTER - ((min8 * SAMPLE_AREA_HEIGHT) >> 8);
        *outMax = SAMPLE_AREA_Y_CENTER - ((max8 * SAMPLE_AREA_HEIGHT) >> 8);
    }
}

void setSampleView(int32_t len)
{
    smpEd_ViewSize = len;
}

void setSamplePos(int32_t pos)
{
    smpEd_ScrPos = pos;
}

void writeWaveform(void)
{
    int16_t x, y1, y2, min, max, oldMin, oldMax;
    int32_t numSmpsPerPixel;

    /* clear sample data area (this has to be *FAST*, so don't use clearRect()) */
    memset(&video.frameBuffer[174 * SCREEN_W], 0, SAMPLE_AREA_WIDTH * SAMPLE_AREA_HEIGHT * sizeof (int32_t));

    /* draw center line */
    hLine(0, SAMPLE_AREA_Y_CENTER, SAMPLE_AREA_WIDTH, PAL_DESKTOP);

    if ((currSmp->pek != NULL) && (currSmp->len > 0) && (smpEd_ViewSize > 0))
    {
        y1 = SAMPLE_AREA_Y_CENTER - getScaledSample(scr2SmpPos(0));

        numSmpsPerPixel = smpEd_ViewSize / SAMPLE_AREA_WIDTH;
        if (numSmpsPerPixel <= 1)
        {
            /* 1:1 or zoomed in */

            for (x = 1; x < SAMPLE_AREA_WIDTH; x++)
            {
                y2 = SAMPLE_AREA_Y_CENTER - getScaledSample(scr2SmpPos(x));
                sampleLine(x - 1, x, y1, y2);
                y1 = y2;
            }
        }
        else
        {
            /* zoomed out */

            oldMin = y1;
            oldMax = y1;

            for (x = 0; x < SAMPLE_AREA_WIDTH; x++)
            {
                getSampleDataPeak(scr2SmpPos(x), numSmpsPerPixel, &min, &max);

                if (x > 0)
                {
                    if (min > oldMax) sampleLine(x - 1, x, oldMax, min);
                    if (max < oldMin) sampleLine(x - 1, x, oldMin, max);
                }

                sampleLine(x, x, max, min);

                oldMin = min;
                oldMax = max;
            }
        }
    }
}

void writeSample(uint8_t forceSmpRedraw)
{
    int32_t tmpRx1, tmpRx2;

    /* update sample loop points for visuals */
    if (currSmp == NULL)
    {
        curSmpRepS = 0;
        curSmpRepL = 0;
    }
    else
    {
        curSmpRepS = currSmp->repS;
        curSmpRepL = currSmp->repL;
    }

    /* exchange range variables if x1 is after x2 */
    if (smpEd_Rx1 > smpEd_Rx2)
    {
        tmpRx2    = smpEd_Rx2;
        smpEd_Rx2 = smpEd_Rx1;
        smpEd_Rx1 = tmpRx2;
    }

    /* clamp range */
    smpEd_Rx1 = CLAMP(smpEd_Rx1, 0, currSmp->len);
    smpEd_Rx2 = CLAMP(smpEd_Rx2, 0, currSmp->len);

    /* sanitize sample scroll position */
    if ((smpEd_ScrPos + smpEd_ViewSize) > currSmp->len)
        smpEd_ScrPos = currSmp->len - smpEd_ViewSize;
 
    if (smpEd_ScrPos < 0)
    {
        smpEd_ScrPos = 0;
        if (smpEd_ViewSize > currSmp->len)
            smpEd_ViewSize = currSmp->len;
    }

    /* handle updating */
    if (editor.ui.sampleEditorShown)
    {
        /* check if we need to redraw sample data */
        if (forceSmpRedraw || ((old_SmpScrPos != smpEd_ScrPos) || (old_ViewSize != smpEd_ViewSize)))
        {
            if (editor.ui.sampleEditorShown)
                writeWaveform();

            old_SmpScrPos = smpEd_ScrPos;
            old_ViewSize  = smpEd_ViewSize;

            if (editor.ui.sampleEditorShown)
                writeRange(); /* range was overwritten, draw it again */

            smpEd_OldSmpPosLine = -1;

            old_Rx1 = smpEd_Rx1;
            old_Rx2 = smpEd_Rx2;
        }

        /* check if we need to write new range */
        if ((old_Rx1 != smpEd_Rx1) || (old_Rx2 != smpEd_Rx2))
        {
            tmpRx1 = smpEd_Rx1;
            tmpRx2 = smpEd_Rx2;

            /* remove old range */
            smpEd_Rx1 = old_Rx1;
            smpEd_Rx2 = old_Rx2;

            if (editor.ui.sampleEditorShown)
                writeRange();

            /* write new range */
            smpEd_Rx1 = tmpRx1;
            smpEd_Rx2 = tmpRx2;

            if (editor.ui.sampleEditorShown)
                writeRange();

            old_Rx1 = smpEd_Rx1;
            old_Rx2 = smpEd_Rx2;
        }

        fixRepeatGadgets();
    }

    if (editor.ui.sampleEditorShown)
        fixSampleDrag();

    updateSampleEditor();
}

void clearRange(void)
{
    smpEd_Rx1 = 0;
    smpEd_Rx2 = 0;
}

void setSampleRange(int32_t start, int32_t end)
{
    if (start < 0) start = 0;
    if (end   < 0) end   = 0;

    /* kludge so that you can mark the last sample of what we see */
    if (end == (SCREEN_W - 1))
    {
        if (smpEd_ViewSize < SAMPLE_AREA_WIDTH) /* zoomed in */
            end += 2;
        else if ((smpEd_ScrPos + smpEd_ViewSize) >= currSmp->len)
            end++;
    }

    smpEd_Rx1 = scr2SmpPos(start);
    smpEd_Rx2 = scr2SmpPos(end);

    /* 2-byte align if sample is 16-bit */
    if (currSmp->typ & 16)
    {
        smpEd_Rx1 &= 0xFFFFFFFE;
        smpEd_Rx2 &= 0xFFFFFFFE;
    }
}

void updateSampleEditorSample(void)
{
    MY_ASSERT(editor.curSmp <= 0x0F)
    currSmp = &instr[editor.curInstr].samp[editor.curSmp];
    MY_ASSERT(currSmp != NULL)

    smpEd_Rx1 = 0;
    smpEd_Rx2 = 0;

    smpEd_ScrPos = 0;
    smpEd_ViewSize = currSmp->len;

    writeSample(true);
}

void updateSampleEditor(void)
{
    const char sharpNote1Char[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };
    const char sharpNote2Char[12] = { '-', '#', '-', '#', '-', '-', '#', '-', '#', '-', '#', '-' };
    const char flatNote1Char[12]  = { 'C', 'D', 'D', 'E', 'E', 'F', 'G', 'G', 'A', 'A', 'B', 'B' };
    const char flatNote2Char[12]  = { '-', 'b', '-', 'b', '-', '-', 'b', '-', 'b', '-', 'b', '-' };
    char noteChar1, noteChar2, octaChar;
    uint8_t note;

    if (!editor.ui.sampleEditorShown)
        return;

    MY_ASSERT(currSmp != NULL)

    /* sample bit depth radio buttons */
    uncheckRadioButtonGroup(RB_GROUP_SAMPLE_DEPTH);
    if (currSmp->typ & 16)
        radioButtons[RB_SAMPLE_16BIT].state = RADIOBUTTON_CHECKED;
    else
        radioButtons[RB_SAMPLE_8BIT].state = RADIOBUTTON_CHECKED;
    showRadioButtonGroup(RB_GROUP_SAMPLE_DEPTH);

    /* sample loop radio buttons */
    uncheckRadioButtonGroup(RB_GROUP_SAMPLE_LOOP);
    if (currSmp->typ & 3)
    {
        if (currSmp->typ & 2)
            radioButtons[RB_SAMPLE_PINGPONG_LOOP].state = RADIOBUTTON_CHECKED;
        else
            radioButtons[RB_SAMPLE_FORWARD_LOOP].state = RADIOBUTTON_CHECKED;
    }
    else
    {
        radioButtons[RB_SAMPLE_NO_LOOP].state = RADIOBUTTON_CHECKED;
    }
    showRadioButtonGroup(RB_GROUP_SAMPLE_LOOP);

    clearRect(7, 369, 22, 8);

    note = (editor.samplerNote - 1) % 12;
    if (config.ptnAcc == 0)
    {
        noteChar1 = sharpNote1Char[note];
        noteChar2 = sharpNote2Char[note];
    }
    else
    {
        noteChar1 = flatNote1Char[note];
        noteChar2 = flatNote2Char[note];
    }

    octaChar = '0' + ((editor.samplerNote - 1) / 12);

    charOut(7,  369, PAL_FORGRND, noteChar1);
    charOut(15, 369, PAL_FORGRND, noteChar2);
    charOut(23, 369, PAL_FORGRND, octaChar);

    fillRect(536, 350, 55, 20, PAL_DESKTOP);
    hexOut(536, 350, PAL_FORGRND, smpEd_ViewSize, 8);
    hexOut(536, 362, PAL_FORGRND, currSmp->len,   8);
}

void sampPlayNoteUp(void)
{
    if (editor.curInstr == 0)
        return;

    if (editor.samplerNote < 96)
    {
        editor.samplerNote++;
        updateSampleEditor();
    }
}

void sampPlayNoteDown(void)
{
    if (editor.curInstr == 0)
        return;

    if (editor.samplerNote > 1)
    {
        editor.samplerNote--;
        updateSampleEditor();
    }
}

void scrollSampleDataLeft(void)
{
    int32_t scrollAmount;

    if ((smpEd_ViewSize > 0) && (smpEd_ViewSize != currSmp->len))
    {
        if (mouse.rightButtonPressed)
        {
            scrollAmount = smpEd_ViewSize / 14; /* rounded from 16 (70Hz) */
            if (scrollAmount < 1)
                scrollAmount = 1;
        }
        else
        {
            scrollAmount = smpEd_ViewSize / 27; /* rounded from 32 (70Hz) */
            if (scrollAmount < 1)
                scrollAmount = 1;
        }

        smpEd_ScrPos -= scrollAmount;
        if (smpEd_ScrPos < 0)
            smpEd_ScrPos = 0;
    }
}

void scrollSampleDataRight(void)
{
    int32_t scrollAmount;

    if ((smpEd_ViewSize > 0) && (smpEd_ViewSize != currSmp->len))
    {
        if (mouse.rightButtonPressed)
        {
            scrollAmount = smpEd_ViewSize / 14; /* rounded from 16 (70Hz) */
            if (scrollAmount < 1)
                scrollAmount = 1;
        }
        else
        {
            scrollAmount = smpEd_ViewSize / 27; /* rounded from 32 (70Hz) */
            if (scrollAmount < 1)
                scrollAmount = 1;
        }

        smpEd_ScrPos += scrollAmount;
        if ((smpEd_ScrPos + smpEd_ViewSize) > currSmp->len)
            smpEd_ScrPos = currSmp->len - smpEd_ViewSize;
    }
}

void scrollSampleData(int32_t pos)
{
    if ((smpEd_ViewSize > 0) && (smpEd_ViewSize != currSmp->len))
        smpEd_ScrPos = pos;
}

void sampPlayWave(void)
{
    if ((editor.curInstr > 0) && (currSmp->pek != NULL))
        playSample(editor.cursor.ch, editor.curInstr, editor.curSmp, editor.samplerNote, 0, 0);
}

void sampPlayRange(void)
{
    if ((editor.curInstr > 0) && (currSmp->pek != NULL) && (smpEd_Rx2 >= 2))
        playRange(editor.cursor.ch, editor.curInstr, editor.curSmp, editor.samplerNote, 0, 0, smpEd_Rx1, smpEd_Rx2);
}

void sampPlayDisplay(void)
{
    if ((editor.curInstr > 0) && (currSmp->pek != NULL) && (smpEd_ViewSize >= 2))
        playRange(editor.cursor.ch, editor.curInstr, editor.curSmp, editor.samplerNote, 0, 0, smpEd_ScrPos, smpEd_ScrPos + smpEd_ViewSize);
}

void showRange(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (smpEd_Rx1 < smpEd_Rx2)
    {
        smpEd_ViewSize = smpEd_Rx2 - smpEd_Rx1;

        if (currSmp->typ & 16)
        {
            if (smpEd_ViewSize < 4)
                smpEd_ViewSize = 4;
        }
        else if (smpEd_ViewSize < 2)
        {
            smpEd_ViewSize = 2;
        }

        smpEd_ScrPos = smpEd_Rx1;
    }
    else
    {
        okBox(0, "System message", "Cannot show empty range!");
    }
}

void rangeAll(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    smpEd_Rx1 = smpEd_ScrPos;
    smpEd_Rx2 = smpEd_ScrPos + smpEd_ViewSize;
}

static void zoomSampleDataIn(int32_t step, int16_t x)
{
    int32_t tmp32, minViewSize;
    int64_t newScrPos64;

    minViewSize = (currSmp->typ & 16) ? 4 : 2;
    if (old_ViewSize <= minViewSize)
        return;

    if (step < 1)
        step = 1;

    smpEd_ViewSize = old_ViewSize - (step * 2);
    if (smpEd_ViewSize < minViewSize)
        smpEd_ViewSize = minViewSize;

    tmp32 = (x - (SAMPLE_AREA_WIDTH / 2)) * step;
    step += (int32_t)(round(tmp32 / (double)(SAMPLE_AREA_WIDTH / 2)));

    newScrPos64 = old_SmpScrPos + step;
    if ((newScrPos64 + smpEd_ViewSize) > currSmp->len)
        newScrPos64 = currSmp->len - smpEd_ViewSize;

    smpEd_ScrPos = newScrPos64 & 0xFFFFFFFF;
}

static void zoomSampleDataOut(int32_t step, int16_t x)
{
    int32_t tmp32;
    int64_t newViewSize64;

    if (old_ViewSize == currSmp->len)
        return;

    if (step < 1)
        step = 1;

    newViewSize64 = (int64_t)(old_ViewSize) + (step * 2);
    if (newViewSize64 > currSmp->len)
    {
        smpEd_ViewSize = currSmp->len;
        smpEd_ScrPos   = 0;
    }
    else
    {
        tmp32 = (x - (SAMPLE_AREA_WIDTH / 2)) * step;
        step += (int32_t)(round(tmp32 / (double)(SAMPLE_AREA_WIDTH / 2)));

        smpEd_ViewSize = newViewSize64 & 0xFFFFFFFF;

        smpEd_ScrPos = old_SmpScrPos - step;
        if (smpEd_ScrPos < 0)
            smpEd_ScrPos = 0;

        if ((smpEd_ScrPos + smpEd_ViewSize) > currSmp->len)
            smpEd_ScrPos = currSmp->len - smpEd_ViewSize;
    }
}

void mouseZoomSampleDataIn(void)
{
    int32_t step;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    step = (int32_t)(round(old_ViewSize / 10.0));
    zoomSampleDataIn(step, mouse.x);
}

void mouseZoomSampleDataOut(void)
{
    int32_t step;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    step = (int32_t)(round(old_ViewSize / 10.0));
    zoomSampleDataOut(step, mouse.x);
}

void zoomOut(void)
{
    if ((old_ViewSize == currSmp->len) || (editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    smpEd_ScrPos = (int32_t)(round(old_SmpScrPos - (old_ViewSize / 2.0)));
    if (smpEd_ScrPos < 0)
        smpEd_ScrPos = 0;

    smpEd_ViewSize = old_ViewSize * 2;
    if (smpEd_ViewSize < old_ViewSize)
    {
        smpEd_ViewSize = currSmp->len;
        smpEd_ScrPos   = 0;
    }
    else if ((smpEd_ViewSize + smpEd_ScrPos) > currSmp->len)
    {
        smpEd_ViewSize = currSmp->len - smpEd_ScrPos;
    }
}

void showAll(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    smpEd_ScrPos   = 0;
    smpEd_ViewSize = currSmp->len;
}

void saveRange(void)
{
    UNICHAR *filenameU;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (smpEd_Rx1 >= smpEd_Rx2)
    {
        okBox(0, "System message", "No range specified!");
        return;
    }

    smpEd_SysReqText[0] = '\0';
    if (inputBox(1, "Enter filename:", smpEd_SysReqText, sizeof (smpEd_SysReqText) - 1) != 1)
        return;

    if (smpEd_SysReqText[0] == '\0')
    {
        okBox(0, "System message", "Filename can't be empty!");
        return;
    }

    if (smpEd_SysReqText[0] == '.')
    {
        okBox(0, "System message", "The very first character in the filename can't be '.' (dot)!");
        return;
    }

    if (strpbrk(smpEd_SysReqText, "\\/:*?\"<>|") != NULL)
    {
        okBox(0, "System message", "The filename can't contain the following characters: \\ / : * ? \" < > |");
        return;
    }

    switch (editor.sampleSaveMode)
    {
                 case SMP_SAVE_MODE_RAW: changeFilenameExt(smpEd_SysReqText, ".raw", sizeof (smpEd_SysReqText) - 1); break;
                 case SMP_SAVE_MODE_IFF: changeFilenameExt(smpEd_SysReqText, ".iff", sizeof (smpEd_SysReqText) - 1); break;
        default: case SMP_SAVE_MODE_WAV: changeFilenameExt(smpEd_SysReqText, ".wav", sizeof (smpEd_SysReqText) - 1); break;
    }

    filenameU = cp437ToUnichar(smpEd_SysReqText);
    if (filenameU == NULL)
    {
        okBox(0, "System message", "Error converting string locale!");
        return;
    }

    saveSample(filenameU, SAVE_RANGE);
    free(filenameU);
}

int8_t cutRange(void)
{
    int32_t len, repE;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return (false);

    MY_ASSERT(!(currSmp->typ & 16) || (!(smpEd_Rx1 & 1) && !(smpEd_Rx2 & 1) && !(currSmp->len & 1)))

    pauseAudio();
    restoreSample(currSmp);

    if (config.smpCutToBuffer)
    {
        if (!getCopyBuffer(smpEd_Rx2 - smpEd_Rx1))
        {
            fixSample(currSmp);
            resumeAudio();

            okBox(0, "System message", "Not enough memory!");
            return (false);
        }

        memcpy(smpCopyBuff, &currSmp->pek[smpEd_Rx1], smpEd_Rx2 - smpEd_Rx1);
        smpCopyBits = (currSmp->typ & 16) ? 16 : 8;
    }

    memcpy(&currSmp->pek[smpEd_Rx1], &currSmp->pek[smpEd_Rx2], currSmp->len - smpEd_Rx2);

    len = currSmp->len - smpEd_Rx2 + smpEd_Rx1;
    if (len > 0)
    {
        currSmp->pek = (int8_t *)(realloc(currSmp->pek, len + 2));
        if (currSmp->pek == NULL)
        {
            freeSample(currSmp);
            writeSample(true);
            resumeAudio();

            okBox(0, "System message", "Not enough memory!");
            return (false);
        }

        currSmp->len = len;

        repE = currSmp->repS + currSmp->repL;
        if (currSmp->repS > smpEd_Rx1)
        {
            currSmp->repS -= (smpEd_Rx2 - smpEd_Rx1);
            if (currSmp->repS < smpEd_Rx1)
                currSmp->repS = smpEd_Rx1;
        }

        if (repE > smpEd_Rx1)
        {
           repE -= (smpEd_Rx2 - smpEd_Rx1);
            if (repE < smpEd_Rx1)
                repE = smpEd_Rx1;
        }

        currSmp->repL = repE - currSmp->repS;
        if (currSmp->repL < 0)
            currSmp->repL = 0;

        if ((currSmp->repS + currSmp->repL) > len)
            currSmp->repL = len - currSmp->repS;

        /* 2-byte align loop points if sample is 16-bit */
        if (currSmp->typ & 16)
        {
            currSmp->repL &= 0xFFFFFFFE;
            currSmp->repS &= 0xFFFFFFFE;
        }

        if (currSmp->repL == 0)
        {
            currSmp->repS = 0;
            currSmp->typ &= 0xFC; /* disable loop */
        }

        fixSample(currSmp);
    }
    else
    {
        freeSample(currSmp);
    }

    resumeAudio();

    setSongModifiedFlag();

    smpEd_Rx2 = smpEd_Rx1;
    writeSample(true);

    return (true);
}

void sampCut(void)
{
    if ((editor.curInstr == 0) || (smpEd_Rx2 == 0) || (smpEd_Rx2 < smpEd_Rx1) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    if (!cutRange())
        okBox(0, "System message", "Not enough memory! (Disable \"cut to buffer\")");
    else
        writeSample(true);
}

void sampCopy(void)
{
    if ((editor.curInstr == 0) || (smpEd_Rx2 == 0) || (smpEd_Rx2 < smpEd_Rx1) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    MY_ASSERT(!(currSmp->typ & 16) || (!(smpEd_Rx1 & 1) && !(smpEd_Rx2 & 1)))

    if (!getCopyBuffer(smpEd_Rx2 - smpEd_Rx1))
    {
        okBox(0, "System message", "Not enough memory!");
        return;
    }

    restoreSample(currSmp);
    memcpy(smpCopyBuff, &currSmp->pek[smpEd_Rx1], smpEd_Rx2 - smpEd_Rx1);
    fixSample(currSmp);

    smpCopyBits = (currSmp->typ & 16) ? 16 : 8;
}

void sampPaste(void)
{
    int8_t *ptr, *ptr8;
    int16_t *ptr16;
    int32_t i, l, d, realCopyLen;

    if ((editor.curInstr == 0) || (smpEd_Rx2 < smpEd_Rx1) || (smpCopyBuff == NULL) || (smpCopySize == 0))
        return;

    /* paste without selecting where (overwrite) */
    if (smpEd_Rx2 == 0)
    {
        ptr = (int8_t *)(malloc(smpCopySize + 2));
        if (ptr == NULL)
        {
            okBox(0, "System message", "Not enough memory!");
            return;
        }

        pauseAudio();
        restoreSample(currSmp);

        if (currSmp->pek != NULL)
            free(currSmp->pek);

        memcpy(ptr, smpCopyBuff, smpCopySize);

        currSmp->pek    = ptr;
        currSmp->len    = smpCopySize;
        currSmp->repL   = 0;
        currSmp->repS   = 0;
        currSmp->vol    = 64;
        currSmp->pan    = 128;
        currSmp->relTon = 0;
        currSmp->fine   = 0;
        currSmp->typ    = (smpCopyBits == 16) ? 16 : 0;

        fixSample(currSmp);
        resumeAudio();

        updateSampleEditorSample();
        setSongModifiedFlag();

        return;
    }

    MY_ASSERT(!(currSmp->typ & 16) || (!(smpEd_Rx1 & 1) && !(smpEd_Rx2 & 1) && !(currSmp->len & 1)))

    realCopyLen = smpCopySize;
    if (currSmp->pek != NULL)
    {
        if (!(currSmp->typ & 16))
        {
            /* destination sample is 8-bit */

            /* copy buffer is 16-bit, divide by 2 */
            if (smpCopyBits == 16)
            {
                realCopyLen &= 0xFFFFFFFE;
                realCopyLen /= 2;
            }
        }
        else
        {
            /* destination sample is 16-bit */

            /* copy buffer is 8-bit, multiply by 2 */
            if (smpCopyBits == 8)
                realCopyLen *= 2;
        }

        d = realCopyLen - (smpEd_Rx1 - smpEd_Rx2);
        l = currSmp->len + realCopyLen - (smpEd_Rx2 - smpEd_Rx1);
    }
    else
    {
        /* destination sample is empty */
        d = 0;
        l = smpCopySize;
    }

    ptr = (int8_t *)(malloc(l + 2));
    if (ptr == NULL)
    {
        okBox(0, "System message", "Not enough memory!");
        return;
    }

    pauseAudio();
    restoreSample(currSmp);

    if (currSmp->pek != NULL)
    {
        /* copy first part of sample (left side before copy part) */
        memcpy(ptr, currSmp->pek, smpEd_Rx1);

        if (currSmp->typ & 16)
        {
            /* destination sample = 16-bit */

            if (smpCopyBits == 16)
            {
                /* src/dst = equal bits, copy directly */
                memcpy(&ptr[smpEd_Rx1], smpCopyBuff, realCopyLen);
            }
            else
            {
                /* convert copy data to 16-bit then paste */
                ptr16 = (int16_t *)(&ptr[smpEd_Rx1]);
                for (i = 0; i < (realCopyLen / 2); ++i)
                    ptr16[i] = smpCopyBuff[i] << 8;
            }
        }
        else
        {
            /* destination sample = 8-bit */

            if (smpCopyBits == 8)
            {
                /* src/dst = equal bits, copy directly */
                memcpy(&ptr[smpEd_Rx1], smpCopyBuff, realCopyLen);
            }
            else
            {
                /* convert copy data to 8-bit then paste */
                ptr8  = (int8_t  *)(&ptr[smpEd_Rx1]);
                ptr16 = (int16_t *)(smpCopyBuff);

                for (i = 0; i < realCopyLen; ++i)
                    ptr8[i] = (int8_t)(ptr16[i] >> 8);
            }
        }

        /* copy remaining data from original sample */
        memcpy(&ptr[smpEd_Rx1 + realCopyLen], &currSmp->pek[smpEd_Rx2], currSmp->len - smpEd_Rx2);
        free(currSmp->pek);

        /* adjust loop points if necessary */
        if ((smpEd_Rx2 - smpEd_Rx1) != realCopyLen)
        {
            if (currSmp->repS > smpEd_Rx2)
            {
                currSmp->repS += d;
                currSmp->repL -= d;
            }

            if ((currSmp->repS + currSmp->repL) > smpEd_Rx2)
                currSmp->repL += d;

            if (currSmp->repS > l)
            {
                currSmp->repS = 0;
                currSmp->repL = 0;
            }

            if ((currSmp->repS + currSmp->repL) > l)
                currSmp->repL = l - currSmp->repS;

            /* 2-byte align loop points if smaple is 16-bit */
            if (currSmp->typ & 16)
            {
                currSmp->repL &= 0xFFFFFFFE;
                currSmp->repS &= 0xFFFFFFFE;
            }
        }
    }
    else
    {
        /* we pasted to an empty sample */

        /* copy over copy buffer data */
        memcpy(ptr, smpCopyBuff, smpCopySize);

        /* set new sample bit depth */
        if (smpCopyBits == 16)
            currSmp->typ |= 16;
        else
            currSmp->typ &= ~16;

        smpEd_ViewSize = l;
    }

    currSmp->len = l;
    currSmp->pek = ptr;

    fixSample(currSmp);
    resumeAudio();

    /* set new range */
    smpEd_Rx2 = smpEd_Rx1 + realCopyLen;

    if (currSmp->typ & 16)
    {
        smpEd_Rx1 &= 0xFFFFFFFE;
        smpEd_Rx2 &= 0xFFFFFFFE;
    }

    writeSample(true);
    setSongModifiedFlag();
}

void sampCrop(void)
{
    int8_t tb;
    int32_t r1, r2;

    if ((smpEd_Rx1 >= smpEd_Rx2) || (editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    MY_ASSERT(!(currSmp->typ & 16) || (!(smpEd_Rx1 & 1) && !(smpEd_Rx2 & 1) && !(currSmp->len & 1)))

    r1 = smpEd_Rx1;
    r2 = smpEd_Rx2;
    tb = config.smpCutToBuffer;

    config.smpCutToBuffer = false;

    pauseAudio();

    smpEd_Rx1 = 0;
    smpEd_Rx2 = r1;
    cutRange();

    smpEd_Rx1 = r2 - r1;
    smpEd_Rx2 = currSmp->len;
    cutRange();

    resumeAudio();

    smpEd_Rx1 = 0;
    smpEd_Rx2 = currSmp->len;

    if (currSmp->typ & 16)
    {
        smpEd_Rx1 &= 0xFFFFFFFE;
        smpEd_Rx2 &= 0xFFFFFFFE;
    }

    config.smpCutToBuffer = tb;

    writeSample(true);
    setSongModifiedFlag();
}

void sampVolume(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;
}

void sampXFade(void)
{
    int8_t is16Bit;
    uint8_t t;
    int16_t c ,d;
    int32_t i, x1, x2, y1, y2, a, b, d1, d2, d3, dist;
    double dR, dS1, dS2, dS3, dS4;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    MY_ASSERT(!(currSmp->typ & 16) || (!(smpEd_Rx1 & 1) && !(smpEd_Rx2 & 1) && !(currSmp->len & 1)))

    t = currSmp->typ;

    /* check if the sample has the loop flag enabled */
    if ((t & 3) == 0)
    {
        okBox(0, "System message", "X-Fade can only be used on a loop-enabled sample!");
        return;
    }

    /* check if we selected a range */
    if (smpEd_Rx2 == 0)
    {
        okBox(0, "System message", "No range selected! Make a small range that includes loop start or loop end.");
        return;
    }

    /* check if we selected a valid range length */
    if ((smpEd_Rx2 - smpEd_Rx1) <= 2)
    {
        okBox(0, "System message", "Invalid range!");
        return;
    }

    x1 = smpEd_Rx1;
    x2 = smpEd_Rx2;

    is16Bit = (t & 16) ? true : false;

    if ((t & 3) >= 2)
    {
        /* bidi loop */

        y1 = currSmp->repS;
        if (x1 <= y1)
        {
            /* first loop point */

            if ((x2 <= y1) || (x2 >= (currSmp->repS + currSmp->repL)))
            {
                okBox(0, "System message", "Not enough sample data outside loop!");
                return;
            }

            d1 = y1 - x1;
            if ((x2 - y1) > d1)
                d1 = x2 - y1;

            d2 = y1 - x1;
            d3 = x2 - y1;

            if ((d1 < 1) || (d2 < 1) || (d3 < 1))
            {
                okBox(0, "System message", "Not enough sample data outside loop!");
                return;
            }

            if (((y1 - d1) < 0) || ((y1 + d1) >= currSmp->len))
            {
                okBox(0, "System message", "Invalid range!");
                return;
            }

            dist = 1;
            if (is16Bit)
                dist++;

            pauseAudio();
            restoreSample(currSmp);

            i = 0;
            while (i < d1)
            {
                a = getSampleValueNr(currSmp->pek, t, y1 + dist * (-i - 1));
                b = getSampleValueNr(currSmp->pek, t, y1 + dist * i);

                dS1 = 1.0 - i / (double)(d2); dS2 = 2.0 - dS1;
                dS3 = 1.0 - i / (double)(d3); dS4 = 2.0 - dS3;

                c = (int16_t)(round((a * dS2 + b * dS1) / (dS1 + dS2)));
                d = (int16_t)(round((b * dS4 + a * dS3) / (dS3 + dS4)));

                if (i < d2) putSampleValueNr(currSmp->pek, t, y1 + dist * (-i - 1), c);
                if (i < d3) putSampleValueNr(currSmp->pek, t, y1 + dist * i, d);

                i += dist;
            }

            fixSample(currSmp);
            resumeAudio();
        }
        else
        {
            /* last loop point */

            y1 += currSmp->repL;
            if ((x1 >= y1) || (x2 <= y1) || (x2 >= currSmp->len))
            {
                okBox(0, "System message", "Not enough sample data outside loop!");
                return;
            }

            d1 = y1 - x1;
            if ((x2 - y1) > d1)
                d1 = x2 - y1;

            d2 = y1 - x1;
            d3 = x2 - y1;

            if ((d1 < 1) || (d2 < 1) || (d3 < 1))
            {
                okBox(0, "System message", "Not enough sample data outside loop!");
                return;
            }

            if (((y1 - d1) < 0) || ((y1 + d1) >= currSmp->len))
            {
                okBox(0, "System message", "Invalid range!");
                return;
            }

            dist = 1;
            if (is16Bit)
                dist++;

            pauseAudio();
            restoreSample(currSmp);

            i = 0;
            while (i < d1)
            {
                a = getSampleValueNr(currSmp->pek, t, y1 - i - dist);
                b = getSampleValueNr(currSmp->pek, t, y1 + i);

                dS1 = 1.0 - i / (double)(d2); dS2 = 2.0 - dS1;
                dS3 = 1.0 - i / (double)(d3); dS4 = 2.0 - dS3;

                c = (int16_t)(round((a * dS2 + b * dS1) / (dS1 + dS2)));
                d = (int16_t)(round((b * dS4 + a * dS3) / (dS3 + dS4)));

                if (i < d2) putSampleValueNr(currSmp->pek, t, y1 - i - dist, c);
                if (i < d3) putSampleValueNr(currSmp->pek, t, y1 + i, d);

                i += dist;
            }

            fixSample(currSmp);
            resumeAudio();
        }
    }
    else
    {
        /* standard loop */

        if (x1 > currSmp->repS)
        {
            x1 -= currSmp->repL;
            x2 -= currSmp->repL;
        }

        if ((x1 < 0) || (x2 <= x1) || (x2 >= currSmp->len))
        {
            okBox(0, "System message", "Not enough sample data outside loop!");
            return;
        }

        i = (x2 - x1 + 1) / 2;
        y1 = currSmp->repS - i;
        y2 = currSmp->repS + currSmp->repL - i;

        if (t & 16)
        {
            y1 &= 0xFFFFFFFE;
            y2 &= 0xFFFFFFFE;
        }

        if ((y1 < 0) || ((y2 + (x2 - x1)) >= currSmp->len))
        {
            okBox(0, "System message", "Invalid range!");
            return;
        }

        d1 = x2 - x1;
        d2 = currSmp->repS - y1;
        d3 = x2 - x1 - d2;

        if (((y1 + (x2 - x1)) <= currSmp->repS) || (d1 == 0) || (d3 == 0) || (d1 > currSmp->repL))
        {
            okBox(0, "System message", "Not enough sample data outside loop!");
            return;
        }

        dR = (currSmp->repS - i) / (double)(x2 - x1);

        pauseAudio();
        restoreSample(currSmp);

        i = 0;
        while (i < (x2 - x1))
        {
            a = getSampleValueNr(currSmp->pek, t, y1 + i);
            b = getSampleValueNr(currSmp->pek, t, y2 + i);

            dS2 = i / (double)(d1);
            dS1 = 1.0 - dS2;

            if ((y1 + i) < currSmp->repS)
            {
                dS3 = 1.0 - (1.0 - dR) * i / d2;
                dS4 = dR * i / d2;

                c = (int16_t)(round((a * dS3 + b * dS4) / (dS3 + dS4)));
                d = (int16_t)(round((a * dS2 + b * dS1) / (dS1 + dS2)));
            }
            else
            {
                dS3 = 1.0 - (1.0 - dR) * (d1 - i) / d3;
                dS4 = dR * (d1 - i) / d3;

                c = (int16_t)(round((a * dS2 + b * dS1) / (dS1 + dS2)));
                d = (int16_t)(round((a * dS4 + b * dS3) / (dS3 + dS4)));
            }

            putSampleValueNr(currSmp->pek, t, y1 + i, c);
            putSampleValueNr(currSmp->pek, t, y2 + i, d);

            i += (is16Bit ? 2 : 1);
        }

        fixSample(currSmp);
        resumeAudio();
    }

    writeSample(true);
    setSongModifiedFlag();
}

void rbSampleNoLoop(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    lockMixerCallback();
    restoreSample(currSmp);

    currSmp->typ &= ~3;

    fixSample(currSmp);
    unlockMixerCallback();

    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void rbSampleForwardLoop(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    lockMixerCallback();
    restoreSample(currSmp);

    currSmp->typ = (currSmp->typ & ~3) | 1;

    if ((currSmp->repL + currSmp->repS) == 0)
    {
        currSmp->repS = 0;
        currSmp->repL = currSmp->len;
    }

    fixSample(currSmp);
    unlockMixerCallback();

    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void rbSamplePingpongLoop(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    lockMixerCallback();
    restoreSample(currSmp);

    currSmp->typ = (currSmp->typ & ~3) | 2;

    if ((currSmp->repL + currSmp->repS) == 0)
    {
        currSmp->repS = 0;
        currSmp->repL = currSmp->len;
    }

    fixSample(currSmp);
    unlockMixerCallback();

    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void rbSample8bit(void)
{
    int8_t *dst8;
    int16_t *src16;
    int32_t i, newLen;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    radioButtons[RB_SAMPLE_8BIT].state  = RADIOBUTTON_CHECKED;
    radioButtons[RB_SAMPLE_16BIT].state = RADIOBUTTON_UNCHECKED;
    drawRadioButton(RB_SAMPLE_8BIT);
    drawRadioButton(RB_SAMPLE_16BIT);

    if (okBox(2, "System request", "Convert sampledata?") == 1)
    {
        newLen = currSmp->len / 2;

        dst8 = (int8_t *)(malloc(newLen + 2));
        if (dst8 == NULL)
        {
            okBox(0, "System message", "Not enough memory!");
            return;
        }

        pauseAudio();
        restoreSample(currSmp);

        currSmp->typ &= ~16; /* remove 16-bit flag */

        currSmp->repL /= 2;
        currSmp->repS /= 2;
        currSmp->len  /= 2;

        src16 = (int16_t *)(currSmp->pek);
        for (i = 0; i < newLen; ++i)
            dst8[i] = (int8_t)(src16[i] >> 8);

        free(currSmp->pek);
        currSmp->pek = dst8;

        fixSample(currSmp);
        resumeAudio();
    }
    else
    {
        lockMixerCallback();
        restoreSample(currSmp);

        currSmp->typ &= ~16; /* remove 16-bit flag */

        fixSample(currSmp);
        unlockMixerCallback();
    }

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void rbSample16bit(void)
{
    int8_t *dst8, *src8;
    int16_t *dst16;
    int32_t i, newLen;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    radioButtons[RB_SAMPLE_16BIT].state = RADIOBUTTON_CHECKED;
    radioButtons[RB_SAMPLE_8BIT].state  = RADIOBUTTON_UNCHECKED;
    drawRadioButton(RB_SAMPLE_8BIT);
    drawRadioButton(RB_SAMPLE_16BIT);

    if (okBox(2, "System request", "Convert sampledata?") == 1)
    {
        pauseAudio();
        restoreSample(currSmp);

        newLen = currSmp->len * 2;

        dst8 = (int8_t *)(malloc(newLen + 2));
        if (dst8 == NULL)
        {
            okBox(0, "System message", "Not enough memory!");
            return;
        }

        currSmp->typ |= 16; /* add 16-bit flag */

        currSmp->repL *= 2;
        currSmp->repS *= 2;

        dst16 = (int16_t *)(dst8);
        src8  = currSmp->pek;

        for (i = 0; i < currSmp->len; ++i)
            dst16[i] = src8[i] << 8;

        currSmp->len = newLen;

        free(currSmp->pek);
        currSmp->pek = dst8;

        fixSample(currSmp);
        resumeAudio();
    }
    else
    {
        lockMixerCallback();
        restoreSample(currSmp);

        currSmp->typ |= 16; /* add 16-bit flag */

        /* make sure stuff is 2-byte aligned for 16-bit mode */
        currSmp->repS &= 0xFFFFFFFE;
        currSmp->repL &= 0xFFFFFFFE;
        currSmp->len  &= 0xFFFFFFFE;

        fixSample(currSmp);
        unlockMixerCallback();
    }

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void clearSample(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (okBox(1, "System request", "Clear sample?") != 1)
        return;

    lockMixerCallback();

    if (currSmp->pek != NULL)
        free(currSmp->pek);

    memset(currSmp, 0, sizeof (sampleTyp));
    currSmp->vol = 64;
    currSmp->pan = 128;

    unlockMixerCallback();

    updateNewSample();
    setSongModifiedFlag();
}

void sampMin(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (!(currSmp->typ & 3) || ((currSmp->repS + currSmp->repL) >= currSmp->len) || ((currSmp->repS + currSmp->repL) <= 0))
    {
        okBox(0, "System message", "Sample is already minimized.");
    }
    else
    {
        lockMixerCallback();
        restoreSample(currSmp);

        currSmp->len = currSmp->repS + currSmp->repL;

        currSmp->pek = (int8_t *)(realloc(currSmp->pek, currSmp->len + 2));
        if (currSmp->pek == NULL)
        {
            freeSample(currSmp);
            okBox(0, "System message", "Not enough memory!");
        }

        fixSample(currSmp);
        unlockMixerCallback();

        updateSampleEditorSample();
        updateSampleEditor();
        setSongModifiedFlag();
    }
}

void sampRepeatUp(void)
{
    int32_t repS, repL, i, addVal, loopLen, lenSub;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    if (currSmp->typ & 16)
    {
        lenSub = 4;
        addVal = 2;
    }
    else
    {
        lenSub  = 2;
        addVal  = 1;
    }

    repS = curSmpRepS;
    repL = curSmpRepL;

    loopLen = mouse.rightButtonPressed ? 16 : 1;
    for (i = 0; i < loopLen; ++i)
    {
        if (repS < (currSmp->len - lenSub))
            repS += addVal;

        if ((repS + repL) > currSmp->len)
            repL = currSmp->len - repS;
    }

    curSmpRepS = (currSmp->typ & 16) ? (signed)(repS & 0xFFFFFFFE) : repS;
    curSmpRepL = (currSmp->typ & 16) ? (signed)(repL & 0xFFFFFFFE) : repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void sampRepeatDown(void)
{
    int32_t repS, delta;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    delta = mouse.rightButtonPressed ? 16 : 1;
    if (currSmp->typ & 16)
        delta *= 2;

    repS = curSmpRepS - delta;
    if (repS < 0)
        repS = 0;

    curSmpRepS = (currSmp->typ & 16) ? (signed)(repS & 0xFFFFFFFE) : repS;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void sampReplenUp(void)
{
    int32_t repL, delta;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    delta = mouse.rightButtonPressed ? 16 : 1;
    if (currSmp->typ & 16)
        delta *= 2;

    repL = curSmpRepL + delta;
    if ((curSmpRepS + repL) > currSmp->len)
        repL = currSmp->len - curSmpRepS;

    curSmpRepL = (currSmp->typ & 16) ? (signed)(repL & 0xFFFFFFFE) : repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void sampReplenDown(void)
{
    int32_t repL, delta;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    delta = mouse.rightButtonPressed ? 16 : 1;
    if (currSmp->typ & 16)
        delta *= 2;

    repL = curSmpRepL - delta;
    if (repL < 0)
        repL = 0;

    curSmpRepL = (currSmp->typ & 16) ? (signed)(repL & 0xFFFFFFFE) : repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void hideSampleEditor(void)
{
    hidePushButton(PB_SAMP_SCROLL_LEFT);
    hidePushButton(PB_SAMP_SCROLL_RIGHT);
    hidePushButton(PB_SAMP_PNOTE_UP);
    hidePushButton(PB_SAMP_PNOTE_DOWN);
    hidePushButton(PB_SAMP_STOP);
    hidePushButton(PB_SAMP_PWAVE);
    hidePushButton(PB_SAMP_PRANGE);
    hidePushButton(PB_SAMP_PDISPLAY);
    hidePushButton(PB_SAMP_SHOW_RANGE);
    hidePushButton(PB_SAMP_RANGE_ALL);
    hidePushButton(PB_SAMP_CLR_RANGE);
    hidePushButton(PB_SAMP_ZOOM_OUT);
    hidePushButton(PB_SAMP_SHOW_ALL);
    hidePushButton(PB_SAMP_SAVE_RNG);
    hidePushButton(PB_SAMP_CUT);
    hidePushButton(PB_SAMP_COPY);
    hidePushButton(PB_SAMP_PASTE);
    hidePushButton(PB_SAMP_CROP);
    hidePushButton(PB_SAMP_VOLUME);
    hidePushButton(PB_SAMP_XFADE);
    hidePushButton(PB_SAMP_EXIT);
    hidePushButton(PB_SAMP_CLEAR);
    hidePushButton(PB_SAMP_MIN);
    hidePushButton(PB_SAMP_REPEAT_UP);
    hidePushButton(PB_SAMP_REPEAT_DOWN);
    hidePushButton(PB_SAMP_REPLEN_UP);
    hidePushButton(PB_SAMP_REPLEN_DOWN);

    hideRadioButtonGroup(RB_GROUP_SAMPLE_LOOP);
    hideRadioButtonGroup(RB_GROUP_SAMPLE_DEPTH);

    hideScrollBar(SB_SAMP_SCROLL);

    editor.ui.sampleEditorShown = false;

    hideSprite(SPRITE_LEFT_LOOP_PIN);
    hideSprite(SPRITE_RIGHT_LOOP_PIN);
}

void exitSampleEditor(void)
{
    hideSampleEditor();

    if (editor.ui.sampleEditorExtShown)
        hideSampleEditorExt();

    showPatternEditor();
}

void showSampleEditor(void)
{
    if (editor.ui.extended)
        exitPatternEditorExtended();

    hideInstEditor();
    hidePatternEditor();
    editor.ui.sampleEditorShown = true;

    drawFramework(0,   329, 632, 17, FRAMEWORK_TYPE1);
    drawFramework(0,   346, 115, 54, FRAMEWORK_TYPE1);
    drawFramework(115, 346, 133, 54, FRAMEWORK_TYPE1);
    drawFramework(248, 346,  49, 54, FRAMEWORK_TYPE1);
    drawFramework(297, 346,  56, 54, FRAMEWORK_TYPE1);
    drawFramework(353, 346,  74, 54, FRAMEWORK_TYPE1);
    drawFramework(427, 346, 205, 54, FRAMEWORK_TYPE1);
    drawFramework(2,   366,  34, 15, FRAMEWORK_TYPE2);

    textOutShadow(5,   352, PAL_FORGRND, PAL_DSKTOP2, "Play:");
    textOutShadow(371, 352, PAL_FORGRND, PAL_DSKTOP2, "No loop");
    textOutShadow(371, 369, PAL_FORGRND, PAL_DSKTOP2, "Forward");
    textOutShadow(371, 386, PAL_FORGRND, PAL_DSKTOP2, "Pingpong");
    textOutShadow(446, 369, PAL_FORGRND, PAL_DSKTOP2, "8-bit");
    textOutShadow(445, 385, PAL_FORGRND, PAL_DSKTOP2, "16-bit");
    textOutShadow(488, 349, PAL_FORGRND, PAL_DSKTOP2, "Display");
    textOutShadow(488, 361, PAL_FORGRND, PAL_DSKTOP2, "Length");
    textOutShadow(488, 375, PAL_FORGRND, PAL_DSKTOP2, "Repeat");
    textOutShadow(488, 387, PAL_FORGRND, PAL_DSKTOP2, "Replen.");

    showPushButton(PB_SAMP_SCROLL_LEFT);
    showPushButton(PB_SAMP_SCROLL_RIGHT);
    showPushButton(PB_SAMP_PNOTE_UP);
    showPushButton(PB_SAMP_PNOTE_DOWN);
    showPushButton(PB_SAMP_STOP);
    showPushButton(PB_SAMP_PWAVE);
    showPushButton(PB_SAMP_PRANGE);
    showPushButton(PB_SAMP_PDISPLAY);
    showPushButton(PB_SAMP_SHOW_RANGE);
    showPushButton(PB_SAMP_RANGE_ALL);
    showPushButton(PB_SAMP_CLR_RANGE);
    showPushButton(PB_SAMP_ZOOM_OUT);
    showPushButton(PB_SAMP_SHOW_ALL);
    showPushButton(PB_SAMP_SAVE_RNG);
    showPushButton(PB_SAMP_CUT);
    showPushButton(PB_SAMP_COPY);
    showPushButton(PB_SAMP_PASTE);
    showPushButton(PB_SAMP_CROP);
    showPushButton(PB_SAMP_VOLUME);
    showPushButton(PB_SAMP_XFADE);
    showPushButton(PB_SAMP_EXIT);
    showPushButton(PB_SAMP_CLEAR);
    showPushButton(PB_SAMP_MIN);
    showPushButton(PB_SAMP_REPEAT_UP);
    showPushButton(PB_SAMP_REPEAT_DOWN);
    showPushButton(PB_SAMP_REPLEN_UP);
    showPushButton(PB_SAMP_REPLEN_DOWN);

    showRadioButtonGroup(RB_GROUP_SAMPLE_LOOP);
    showRadioButtonGroup(RB_GROUP_SAMPLE_DEPTH);

    showScrollBar(SB_SAMP_SCROLL);

    /* clear two lines that are never written to when the sampler is open */
    hLine(0, 173, SAMPLE_AREA_WIDTH, PAL_BCKGRND);
    hLine(0, 328, SAMPLE_AREA_WIDTH, PAL_BCKGRND);

    updateSampleEditor();
    writeSample(true);
}

void toggleSampleEditor(void)
{
    hideInstEditor();

    if (editor.ui.sampleEditorShown)
    {
        exitSampleEditor();
    }
    else
    {
        hidePatternEditor();
        showSampleEditor();
    }
}

static void writeSmpXORLine(int32_t x)
{
    int16_t y;
    uint32_t *ptr32;

    if ((x < 0) || (x >= SCREEN_W))
        return;

    ptr32 = &video.frameBuffer[(174 * SCREEN_W) + x];
    for (y = 0; y < SAMPLE_AREA_HEIGHT; ++y)
    {
        *ptr32 = video.palette[(*ptr32 >> 24) ^ 1]; /* ">> 24" to get palette from pixel, XOR 1 to switch between normal/inverted mode */
        ptr32 += SCREEN_W;
    }
}

static void writeSamplePosLine(void)
{
    int32_t scrPos;
    stmTyp *ch;

#ifdef _DEBUG
    if ((editor.curSmpChannel < 0) || (editor.curSmpChannel >= MAX_VOICES))
        return;
#endif

    ch = &stm[editor.curSmpChannel];
    if ((ch->instrNr == editor.curInstr) && (ch->sampleNr == editor.curSmp))
    {
        scrPos = getSamplePosition(editor.curSmpChannel);

        /* convert sample position to screen position */
        if (scrPos != -1)
            scrPos = smpPos2Scr(scrPos);

        if (scrPos != smpEd_OldSmpPosLine)
        {
            writeSmpXORLine(smpEd_OldSmpPosLine);
            writeSmpXORLine(scrPos);
        }
    }
    else
    {
        writeSmpXORLine(smpEd_OldSmpPosLine);
        scrPos = -1;
    }

    smpEd_OldSmpPosLine = scrPos;
}

void handleSamplerRedrawing(void)
{
    /* update sample editor */

    if (editor.ui.sampleEditorShown)
    {
        if (editor.samplingAudioFlag)
            return;

        if ((smpEd_Rx1    != old_Rx1)       || (smpEd_Rx2      != old_Rx2) ||
            (smpEd_ScrPos != old_SmpScrPos) || (smpEd_ViewSize != old_ViewSize))
        {
            writeSample(false);
        }

        writeSamplePosLine();
    }
}

void setLeftLoopPinPos(int32_t x)
{
    int32_t repS, repL, newPos;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    newPos = scr2SmpPos(x) - curSmpRepS;

    repS = curSmpRepS + newPos;
    repL = curSmpRepL - newPos;

    if (repS < 0)
    {
        repL += repS;
        repS  = 0;
    }

    if (repL < 0)
    {
        repL = 0;
        repS = curSmpRepS + curSmpRepL;
    }

    if (currSmp->typ & 16)
    {
        repS &= 0xFFFFFFFE;
        repL &= 0xFFFFFFFE;
    }

    curSmpRepS = repS;
    curSmpRepL = repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void setRightLoopPinPos(int32_t x)
{
    int32_t repL;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    repL = scr2SmpPos(x) - curSmpRepS;
    if (repL < 0)
        repL = 0;

    if ((repL + curSmpRepS) > currSmp->len)
        repL = currSmp->len - curSmpRepS;

    if (repL < 0)
        repL = 0;

    if (currSmp->typ & 16)
        repL &= 0xFFFFFFFE;

    curSmpRepL = repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void editSampleData(uint8_t mouseButtonHeld)
{
    int8_t *ptr8;
    int16_t *ptr16;
    int32_t mx, my, tmp32, p, vl, tvl, r, rl, rvl;

    /* ported directly from FT2 and slightly modified */

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len <= 0))
        return;

    mx = mouse.x;
    my = mouse.y;

    if (!mouseButtonHeld)
    {
        lastDrawX = scr2SmpPos(mx);
        if (currSmp->typ & 16)
            lastDrawX /= 2;

        lastDrawY = ((my - 174) * 256) / SAMPLE_AREA_HEIGHT;
        lastDrawY = CLAMP(lastDrawY, 0, 255) ^ 0xFF;

        lastMouseX = mx;
        lastMouseY = my;
    }
    else if ((mx == lastMouseX) && (my == lastMouseY))
    {
        return; /* don't do anything if we didn't move the mouse */
    }

    lastMouseX = mx;
    lastMouseY = my;

    /* kludge so that you can edit the very end of the sample */
    if ((mx == (SCREEN_W - 1)) && ((smpEd_ScrPos + smpEd_ViewSize) >= currSmp->len))
        mx++;

    p = scr2SmpPos(mx);
    if (currSmp->typ & 16)
        p /= 2;

    if (((p != lastDrawX) || (p != lastDrawY)) && !keyb.leftShiftPressed)
    {
        vl = ((my - 174) * 256) / SAMPLE_AREA_HEIGHT;
        vl = CLAMP(vl, 0, 255) ^ 0xFF;
    }
    else
    {
        vl = lastDrawY;
    }

    r = p;
    rvl = vl;

    /* swap x/y if needed */
    if (p > lastDrawX)
    {
        /* swap x */
        tmp32 = p;
        p = lastDrawX;
        lastDrawX = tmp32;

        /* swap y */
        tmp32 = lastDrawY;
        lastDrawY = vl;
        vl = tmp32;
    }

    restoreSample(currSmp);
    if (currSmp->typ & 16)
    {
        /* 16-bit */
        ptr16 = (int16_t *)(currSmp->pek);
        for (rl = p; rl <= lastDrawX; ++rl)
        {
            if ((rl >= 0) && ((rl * 2) < currSmp->len))
            {
                if (p != lastDrawX)
                    tvl = ((vl - lastDrawY) * (rl - p) / (p - lastDrawX) + vl) * 256;
                else
                    tvl = vl * 256;

                ptr16[rl] = (int16_t)(tvl ^ 0x8000);
            }
        }
    }
    else
    {
        /* 8-bit */
        ptr8 = currSmp->pek;
        for (rl = p; rl <= lastDrawX; ++rl)
        {
            if ((rl >= 0) && (rl < currSmp->len))
            {
                if (p != lastDrawX)
                    tvl = (vl - lastDrawY) * (rl - p) / (p - lastDrawX) + vl;
                else
                    tvl = vl;

                ptr8[rl] = (int8_t)(tvl ^ 0x80);
            }
        }
    }
    fixSample(currSmp);

    lastDrawY = rvl;
    lastDrawX = r;

    writeSample(true);
    setSongModifiedFlag();
}

void handleSampleDataMouseDown(int8_t mouseButtonHeld)
{
    int32_t tmp, leftLoopPinPos, rightLoopPinPos;

    if (editor.curInstr == 0)
        return;

    if (!mouseButtonHeld)
    {
        editor.ui.rightLoopPinMoving   = false;
        editor.ui.leftLoopPinMoving    = false;
        editor.ui.sampleDataOrLoopDrag = -1;

        mouseXOffs = 0;
        lastMouseX = mouse.x;
        lastMouseY = mouse.y;

        mouse.lastUsedObjectType = OBJECT_SMPDATA;

        if (mouse.leftButtonPressed)
        {
            /* move loop pins */
            if (mouse.y < 183)
            {
                leftLoopPinPos = getSpritePosX(SPRITE_LEFT_LOOP_PIN);
                if ((mouse.x >= leftLoopPinPos) && (mouse.x <= (leftLoopPinPos + 16)))
                {
                    mouseXOffs = (leftLoopPinPos + 8) - mouse.x;

                    editor.ui.sampleDataOrLoopDrag = true;

                    setLeftLoopPinState(true);
                    lastMouseX = mouse.x;

                    editor.ui.leftLoopPinMoving = true;
                    return;
                }
            }
            else if (mouse.y > 318)
            {
                rightLoopPinPos = getSpritePosX(SPRITE_RIGHT_LOOP_PIN);
                if ((mouse.x >= rightLoopPinPos) && (mouse.x <= (rightLoopPinPos + 16)))
                {
                    mouseXOffs = (rightLoopPinPos + 8) - mouse.x;

                    editor.ui.sampleDataOrLoopDrag = true;

                    setRightLoopPinState(true);
                    lastMouseX = mouse.x;

                    editor.ui.rightLoopPinMoving = true;
                    return;
                }
            }

            /* mark data */
            editor.ui.sampleDataOrLoopDrag = mouse.x;
            lastMouseX = editor.ui.sampleDataOrLoopDrag;
            setSampleRange(editor.ui.sampleDataOrLoopDrag, editor.ui.sampleDataOrLoopDrag);
        }
        else if (mouse.rightButtonPressed)
        {
            /* edit data */
            editor.ui.sampleDataOrLoopDrag = true;
            editSampleData(false);
        }

        return;
    }

    if (mouse.rightButtonPressed)
    {
        editSampleData(true);
        return;
    }

    if (mouse.x != lastMouseX)
    {
        if (mouse.leftButtonPressed)
        {
            if (editor.ui.leftLoopPinMoving)
            {
                lastMouseX = mouse.x;
                setLeftLoopPinPos(mouseXOffs + lastMouseX);
            }
            else if (editor.ui.rightLoopPinMoving)
            {
                lastMouseX = mouse.x;
                setRightLoopPinPos(mouseXOffs + lastMouseX);
            }
            else if (editor.ui.sampleDataOrLoopDrag >= 0)
            {
                /* mark data */

                lastMouseX = mouse.x;
                tmp = lastMouseX;

                     if (lastMouseX  > editor.ui.sampleDataOrLoopDrag) setSampleRange(editor.ui.sampleDataOrLoopDrag, tmp);
                else if (lastMouseX == editor.ui.sampleDataOrLoopDrag) setSampleRange(editor.ui.sampleDataOrLoopDrag, editor.ui.sampleDataOrLoopDrag);
                else if (lastMouseX  < editor.ui.sampleDataOrLoopDrag) setSampleRange(tmp, editor.ui.sampleDataOrLoopDrag);
            }
        }
    }
}

/* SAMPLE EDITOR EXTENSION */

void handleSampleEditorExtRedrawing(void)
{
    fillRect(35,   96, 55, 8, PAL_DESKTOP);
    fillRect(99,   96, 55, 8, PAL_DESKTOP);
    fillRect(99,  110, 55, 8, PAL_DESKTOP);
    fillRect(99,  124, 55, 8, PAL_DESKTOP);
    fillRect(226,  96, 13, 8, PAL_DESKTOP);
    fillRect(274,  96, 13, 8, PAL_DESKTOP);
    fillRect(226, 109, 13, 8, PAL_DESKTOP);
    fillRect(274, 109, 13, 8, PAL_DESKTOP);

    hexOut(35,  96, PAL_FORGRND, smpEd_Rx1,             8);
    hexOut(99,  96, PAL_FORGRND, smpEd_Rx2,             8);
    hexOut(99, 110, PAL_FORGRND, smpEd_Rx2 - smpEd_Rx1, 8);
    hexOut(99, 124, PAL_FORGRND, smpCopySize,           8);

    hexOut(226,  96, PAL_FORGRND, editor.srcInstr, 2);
    hexOut(274,  96, PAL_FORGRND, editor.srcSmp,   2);
    hexOut(226, 109, PAL_FORGRND, editor.curInstr, 2);
    hexOut(274, 109, PAL_FORGRND, editor.curSmp,   2);
}

void drawSampleEditorExt(void)
{
    drawFramework(0,    92, 158, 44, FRAMEWORK_TYPE1);
    drawFramework(0,   136, 158, 37, FRAMEWORK_TYPE1);
    drawFramework(158,  92, 133, 81, FRAMEWORK_TYPE1);

    textOutShadow( 4,  96, PAL_FORGRND, PAL_DSKTOP2, "Rng.:");
    charOutShadow(91,  95, PAL_FORGRND, PAL_DSKTOP2, '-');
    textOutShadow( 4, 109, PAL_FORGRND, PAL_DSKTOP2, "Rangesize");
    textOutShadow( 4, 123, PAL_FORGRND, PAL_DSKTOP2, "Copybuffersize");

    textOutShadow(162,  95, PAL_FORGRND, PAL_DSKTOP2, "Src.instr.");
    textOutShadow(245,  96, PAL_FORGRND, PAL_DSKTOP2, "smp.");
    textOutShadow(162, 109, PAL_FORGRND, PAL_DSKTOP2, "Dest.instr.");
    textOutShadow(245, 109, PAL_FORGRND, PAL_DSKTOP2, "smp.");

    showPushButton(PB_SAMP_EXT_CLEAR_COPYBUF);
    showPushButton(PB_SAMP_EXT_CONV);
    showPushButton(PB_SAMP_EXT_ECHO);
    showPushButton(PB_SAMP_EXT_BACKWARDS);
    showPushButton(PB_SAMP_EXT_CONV_W);
    showPushButton(PB_SAMP_EXT_MORPH);
    showPushButton(PB_SAMP_EXT_COPY_INS);
    showPushButton(PB_SAMP_EXT_COPY_SMP);
    showPushButton(PB_SAMP_EXT_XCHG_INS);
    showPushButton(PB_SAMP_EXT_XCHG_SMP);
    showPushButton(PB_SAMP_EXT_RESAMPLE);
    showPushButton(PB_SAMP_EXT_MIX_SAMPLE);
}

void showSampleEditorExt(void)
{
    hideTopScreen();
    showTopScreen(false);

    if (editor.ui.extended)
        exitPatternEditorExtended();

    if (!editor.ui.sampleEditorShown)
        showSampleEditor();

    editor.ui.sampleEditorExtShown = true;
    editor.ui.scopesShown = false;
    drawSampleEditorExt();
}

void hideSampleEditorExt(void)
{
    editor.ui.sampleEditorExtShown = false;

    hidePushButton(PB_SAMP_EXT_CLEAR_COPYBUF);
    hidePushButton(PB_SAMP_EXT_CONV);
    hidePushButton(PB_SAMP_EXT_ECHO);
    hidePushButton(PB_SAMP_EXT_BACKWARDS);
    hidePushButton(PB_SAMP_EXT_CONV_W);
    hidePushButton(PB_SAMP_EXT_MORPH);
    hidePushButton(PB_SAMP_EXT_COPY_INS);
    hidePushButton(PB_SAMP_EXT_COPY_SMP);
    hidePushButton(PB_SAMP_EXT_XCHG_INS);
    hidePushButton(PB_SAMP_EXT_XCHG_SMP);
    hidePushButton(PB_SAMP_EXT_RESAMPLE);
    hidePushButton(PB_SAMP_EXT_MIX_SAMPLE);

    editor.ui.scopesShown = true;
    drawScopeFramework();
}

void toggleSampleEditorExt(void)
{
    if (editor.ui.sampleEditorExtShown)
        hideSampleEditorExt();
    else
        showSampleEditorExt();
}

void sampleBackwards(void)
{
    int8_t tmp8, *ptrStart, *ptrEnd;
    int16_t tmp16, *ptrStart16, *ptrEnd16;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len < 2))
        return;

    if (currSmp->typ & 16)
    {
        if (smpEd_Rx1 >= smpEd_Rx2)
        {
            ptrStart16 = (int16_t *)( currSmp->pek);
            ptrEnd16   = (int16_t *)(&currSmp->pek[currSmp->len - 2]);
        }
        else
        {
            ptrStart16 = (int16_t *)(&currSmp->pek[smpEd_Rx1]);
            ptrEnd16   = (int16_t *)(&currSmp->pek[smpEd_Rx2 - 2]);
        }

        pauseAudio();
        restoreSample(currSmp);

        while (ptrStart16 < ptrEnd16)
        {
            tmp16         = *ptrStart16;
            *ptrStart16++ = *ptrEnd16;
            *ptrEnd16--   = tmp16;
        }

        fixSample(currSmp);
        resumeAudio();
    }
    else
    {
        if (smpEd_Rx1 >= smpEd_Rx2)
        {
            ptrStart =  currSmp->pek;
            ptrEnd   = &currSmp->pek[currSmp->len - 1];
        }
        else
        {
            ptrStart = &currSmp->pek[smpEd_Rx1];
            ptrEnd   = &currSmp->pek[smpEd_Rx2 - 1];
        }

        pauseAudio();
        restoreSample(currSmp);

        while (ptrStart < ptrEnd)
        {
            tmp8        = *ptrStart;
            *ptrStart++ = *ptrEnd;
            *ptrEnd--   = tmp8;
        }

        fixSample(currSmp);
        resumeAudio();
    }

    writeSample(true);
    setSongModifiedFlag();
}

void sampleConv(void)
{
    int8_t *pek8;
    int16_t *pek16;
    int32_t i, len;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    pauseAudio();
    restoreSample(currSmp);

    if (currSmp->typ & 16)
    {
        len = currSmp->len / 2;

        pek16 = (int16_t *)(currSmp->pek);
        for (i = 0; i < len; ++i)
            pek16[i] ^= 0x8000;
    }
    else
    {
        len = currSmp->len;

        pek8 = currSmp->pek;
        for (i = 0; i < len; ++i)
            pek8[i] ^= 0x80;
    }

    fixSample(currSmp);
    resumeAudio();

    writeSample(true);
    setSongModifiedFlag();
}

void sampleConvW(void)
{
    int8_t *pek8, tmp;
    int32_t i, len;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    pauseAudio();
    restoreSample(currSmp);

    len  = currSmp->len / 2;
    pek8 = currSmp->pek;

    for (i = 0; i < len; ++i)
    {
        tmp     = pek8[0];
        pek8[0] = pek8[1];
        pek8[1] = tmp;

        pek8 += 2;
    }

    fixSample(currSmp);
    resumeAudio();

    if (editor.ui.sampleEditorShown)
    {
        writeSample(true);
        setSongModifiedFlag();
    }
}

void fixDC(void)
{
    int8_t *ptr8;
    int16_t *ptr16;
    int32_t i, len, smpSub, smp;
    int64_t offset;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    offset = 0;
    if (currSmp->typ & 16)
    {
        if (smpEd_Rx1 >= smpEd_Rx2)
        {
            MY_ASSERT(!(currSmp->len & 1))

            ptr16 = (int16_t *)(currSmp->pek);
            len   = currSmp->len / 2;
        }
        else
        {
            MY_ASSERT(!(smpEd_Rx1 & 1))
            MY_ASSERT(!(smpEd_Rx2 & 1))

            ptr16 = (int16_t *)(&currSmp->pek[smpEd_Rx1]);
            len   = (smpEd_Rx2 - smpEd_Rx1) / 2;
        }

        if ((len < 0) || (len > ((signed)(currSmp->len) / 2)))
            return;

        pauseAudio();
        restoreSample(currSmp);

        for (i = 0; i < len; ++i)
            offset += ptr16[i];
        offset /= len;

        smpSub = (int32_t)(offset);
        for (i = 0; i < len; ++i)
        {
           smp = ptr16[i] - smpSub;
           CLAMP16(smp);
           ptr16[i] = (int16_t)(smp);
        }

        fixSample(currSmp);
        resumeAudio();
    }
    else
    {
        if (smpEd_Rx1 >= smpEd_Rx2)
        {
            ptr8 = currSmp->pek;
            len  = currSmp->len;
        }
        else
        {
            ptr8 = &currSmp->pek[smpEd_Rx1];
            len  = smpEd_Rx2 - smpEd_Rx1;
        }

        if ((len < 0) || (len > currSmp->len))
            return;

        pauseAudio();
        restoreSample(currSmp);

        for (i = 0; i < len; ++i)
            offset += ptr8[i];
        offset /= len;

        smpSub = (int32_t)(offset);
        for (i = 0; i < len; ++i)
        {
           smp = ptr8[i] - smpSub;
           CLAMP8(smp);
           ptr8[i] = (int8_t)(smp);
        }

        fixSample(currSmp);
        resumeAudio();
    }

    writeSample(true);
    setSongModifiedFlag();
}

void smpEdStop(void)
{
    /* safely kills all voices */
    lockMixerCallback();
    unlockMixerCallback();
}

void testSmpEdMouseUp(void) /* used for setting new loop points */
{
    if (updateLoopsOnMouseUp)
    {
        updateLoopsOnMouseUp = false;

        if ((currSmp == NULL) || (editor.curInstr == 0))
            return;

        if ((currSmp->repS != curSmpRepS) || (currSmp->repL != curSmpRepL))
        {
            lockMixerCallback();
            restoreSample(currSmp);

            setSongModifiedFlag();

            currSmp->repS = curSmpRepS;
            currSmp->repL = curSmpRepL;

            fixSample(currSmp);
            unlockMixerCallback();

            writeSample(true);
        }
    }
}
