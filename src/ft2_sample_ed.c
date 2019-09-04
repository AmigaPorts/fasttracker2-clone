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
static int8_t *smpCopyBuff, smpEd_RelReSmp, mix_Balance = 50;
static uint8_t echo_AddMemory, updateLoopsOnMouseUp;
static int16_t vol_StartVol = 100, vol_EndVol = 100, echo_nEcho = 1, echo_VolChange = 30;
static int32_t smpEd_OldSmpPosLine = -1; /* must be initialized to -1! */
static int32_t smpEd_Rx1, smpEd_Rx2, smpEd_ViewSize, smpEd_ScrPos, smpCopySize, smpCopyBits;
static int32_t old_Rx1, old_Rx2, old_ViewSize, old_SmpScrPos, echo_Distance = 0x100;
static int32_t lastMouseX, lastMouseY, lastDrawX, lastDrawY, mouseXOffs, curSmpRep, curSmpRepL;
static sampleTyp *currSmp;
static SDL_Thread *echoThread;

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

static int16_t getSampleValueNr(int8_t *ptr, uint8_t typ, int32_t pos)
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

static void putSampleValueNr(int8_t *ptr, uint8_t typ, int32_t pos, int16_t val)
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

void cbEchoAddMemory(void)
{
    echo_AddMemory ^= 1;
}

void sbSetEchoNumPos(int32_t pos)
{
    if (echo_nEcho != pos)
        echo_nEcho = (int16_t)(pos);
}

void sbSetEchoDistPos(int32_t pos)
{
    if (echo_Distance != pos)
        echo_Distance = (int32_t)(pos);
}

void sbSetEchoFadeoutPos(int32_t pos)
{
    if (echo_VolChange != pos)
        echo_VolChange = (int16_t)(pos);
}

void pbEchoNumDown(void)
{
    if (echo_nEcho > 0)
        echo_nEcho--;
}

void pbEchoNumUp(void)
{
    if (echo_nEcho < 1024)
        echo_nEcho++;
}

void pbEchoDistDown(void)
{
    if (echo_Distance > 0)
        echo_Distance--;
}

void pbEchoDistUp(void)
{
    if (echo_Distance < 16384)
        echo_Distance++;
}

void pbEchoFadeoutDown(void)
{
    if (echo_VolChange > 0)
        echo_VolChange--;
}

void pbEchoFadeoutUp(void)
{
    if (echo_VolChange < 100)
        echo_VolChange++;
}

static int32_t SDLCALL createEchoThread(void *ptr)
{
    int8_t *readPtr, *writePtr;
    uint8_t is16Bit;
    int32_t numEchoes, distance, readLen, writeLen, i, j;
    int32_t tmp32, smpOut, smpMul, echoRead, echoCycle, writeIdx;
    double dTmp;

    readLen  = currSmp->len;
    readPtr  = currSmp->pek;
    is16Bit  = (currSmp->typ & 16) ? true : false;
    distance = is16Bit ? (echo_Distance * 32) : (echo_Distance * 16);

    /* calculate real number of echoes */
    j = is16Bit ? 32768 : 128; i = 0;
    while ((i < echo_nEcho) && (j > 0))
    {
        j = (j * echo_VolChange) / 100;
        i++;
    }
    numEchoes = i + 1;

    /* set write length (either original length or full echo length) */
    writeLen = readLen;
    if (echo_AddMemory)
    {
        writeLen += (distance * (numEchoes - 1));
        if (is16Bit)
            writeLen &= 0xFFFFFFFE;
    }

    writePtr = (int8_t *)(malloc(writeLen + 2));
    if (writePtr == NULL)
    {
        setMouseBusy(false);
        sysReqQueue(SR_OOM_ERROR);
        return (false);
    }

    pauseAudio();
    restoreSample(currSmp);

    writeIdx = 0;
    while (writeIdx < writeLen)
    {
        tmp32  = 0;
        smpOut = 0;
        smpMul = 32768;

        echoRead  = writeIdx;
        echoCycle = numEchoes;

        while ((echoRead > 0) && (echoCycle-- > 0))
        {
            if (echoRead < readLen)
            {
                if (is16Bit)
                    tmp32 = *((int16_t *)(&readPtr[echoRead & 0xFFFFFFFE]));
                else
                    tmp32 = readPtr[echoRead] << 8;

                dTmp = (tmp32 * smpMul) / 32768.0;
                double2int32_round(tmp32, dTmp);
                smpOut += tmp32;
            }

            dTmp = (echo_VolChange * smpMul) / 100.0;
            double2int32_round(smpMul, dTmp);
            echoRead -= distance;
        }
        CLAMP16(smpOut);

        if (is16Bit)
        {
            *((int16_t *)(&writePtr[writeIdx & 0xFFFFFFFE])) = (int16_t)(smpOut);
            writeIdx += 2;
        }
        else
        {
            writePtr[writeIdx++] = (int8_t)(smpOut >> 8);
        }
    }

    if (is16Bit)
        writeLen &= 0xFFFFFFFE;

    free(readPtr);
    currSmp->pek = writePtr;
    currSmp->len = writeLen;

    fixSample(currSmp);
    resumeAudio();

    setSongModifiedFlag();

    (void)(ptr); /* prevent compiler warning */

    editor.ui.updateLoadedSample = true; /* also sets mouse busy to false when done */
    return (true);
}

void createEcho(void) /* called from sys. req */
{
    hideSystemRequest();

    if ((editor.curInstr == 0) || (echo_nEcho == 0) || (currSmp->pek == NULL))
        return;

    setMouseBusy(true);
    echoThread = SDL_CreateThread(createEchoThread, "FT2 Clone Sample Echo Thread", NULL);
    if (echoThread == NULL)
    {
        setMouseBusy(false);
        sysReqQueue(SR_THREAD_ERROR);
        return;
    }

    /* don't let thread wait for this thread, let it clean up on its own when done */
    SDL_DetachThread(echoThread);
}

void sampleEcho(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    sysReqQueue(SR_SAMP_ECHO);
}

void drawEchoBox(void)
{
    const int16_t x = 171;
    const int16_t y = 220;
    const int16_t w = 291;
    const int16_t h = 66;
    uint16_t i;
    sysReq_t *sysReq;

    /* main fill */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* outer border */
    vLine(x,         y,         h - 1, PAL_BUTTON1);
    hLine(x + 1,     y,         w - 2, PAL_BUTTON1);
    vLine(x + w - 1, y,         h,     PAL_BUTTON2);
    hLine(x,         y + h - 1, w - 1, PAL_BUTTON2);

    /* inner border */
    vLine(x + 2,     y + 2,     h - 5, PAL_BUTTON2);
    hLine(x + 3,     y + 2,     w - 6, PAL_BUTTON2);
    vLine(x + w - 3, y + 2,     h - 4, PAL_BUTTON1);
    hLine(x + 2,     y + h - 3, w - 4, PAL_BUTTON1);

    textOutShadow(177, 226, PAL_FORGRND, PAL_BUTTON2, "Number of echoes");
    textOutShadow(177, 239, PAL_FORGRND, PAL_BUTTON2, "Echo distance");
    textOutShadow(177, 253, PAL_FORGRND, PAL_BUTTON2, "Fade out");
    textOutShadow(192, 270, PAL_FORGRND, PAL_BUTTON2, "Add memory to sample");

    MY_ASSERT(echo_nEcho <= 1024)

    charOutFast(315 + (0 * 7), 226, PAL_FORGRND, '0' + (echo_nEcho / 1000) % 10);
    charOutFast(315 + (1 * 7), 226, PAL_FORGRND, '0' + (echo_nEcho / 100) % 10);
    charOutFast(315 + (2 * 7), 226, PAL_FORGRND, '0' + (echo_nEcho / 10) % 10);
    charOutFast(315 + (3 * 7), 226, PAL_FORGRND, '0' + (echo_nEcho % 10));

    MY_ASSERT((echo_Distance * 16) <= 262144)

    hexOut(308, 240, PAL_FORGRND, echo_Distance * 16, 5);

    MY_ASSERT(echo_VolChange <= 100)

    charOutFast(312 + (0 * 7), 254, PAL_FORGRND, '0' + (echo_VolChange / 100) % 10);
    charOutFast(312 + (1 * 7), 254, PAL_FORGRND, '0' + (echo_VolChange / 10) % 10);
    charOutFast(312 + (2 * 7), 254, PAL_FORGRND, '0' + (echo_VolChange % 10));
    charOutShadow(313 + (3 * 7), 254, PAL_FORGRND, PAL_BUTTON2, '%');

    /* show/activate push buttons... */
    sysReq = &sysReqs[editor.ui.systemRequestID];
    for (i = 0; i < sysReq->numButtons; ++i)
        showPushButton(sysReq->buttonIDs[i]);

    showCheckBox(CB_SAMP_ECHO_ADD_MEMORY);

    showScrollBar(SB_ECHO_NUM);
    setScrollBarPos(SB_ECHO_NUM, echo_nEcho, false);

    showScrollBar(SB_ECHO_DISTANCE);
    setScrollBarPos(SB_ECHO_DISTANCE, echo_Distance, false);

    showScrollBar(SB_ECHO_FADEOUT);
    setScrollBarPos(SB_ECHO_FADEOUT, echo_VolChange, false);
}

void mixSample(void) /* called from sys. req */
{
    int8_t *dstPtr, *mixPtr, *p, dstRelTone;
    uint8_t mixTyp, dstTyp;
    int32_t smp32, x1, x2, i, dstLen, mixLen, maxLen, dst8Size, max8Size, mix8Size;
    instrTyp *srcIns, *dstIns;
    sampleTyp *srcSmp, *dstSmp;
    double dSmp;

    hideSystemRequest();

    if ((editor.curInstr == editor.srcInstr) && (editor.curSmp == editor.srcSmp))
        return;

    srcIns = &instr[editor.srcInstr];
    dstIns = &instr[editor.curInstr];

    srcSmp = &srcIns->samp[editor.srcSmp];
    dstSmp = &dstIns->samp[editor.curSmp];

    mixLen = srcSmp->len;
    mixPtr = srcSmp->pek;
    mixTyp = srcSmp->typ;

    if (mixPtr == NULL)
    {
        mixLen = 0;
        mixTyp = 0;
    }

    dstLen     = dstSmp->len;
    dstPtr     = dstSmp->pek;
    dstTyp     = dstSmp->typ;
    dstRelTone = dstSmp->relTon;

    if (dstPtr == NULL)
    {
        dstLen = 0;
        dstTyp = mixTyp;
        dstRelTone = srcSmp->relTon;
    }

    mix8Size = (mixTyp & 16) ? (mixLen / 2) : mixLen;
    dst8Size = (dstTyp & 16) ? (dstLen / 2) : dstLen;
    max8Size = (dst8Size > mix8Size) ? dst8Size : mix8Size;
    maxLen   = (dstTyp & 16) ? (max8Size * 2) : max8Size;

    if (maxLen <= 0)
        return;

    p = (int8_t *)(calloc(maxLen + 2, sizeof (int8_t)));
    if (p == NULL)
    {
        sysReqQueue(SR_OOM_ERROR);
        return;
    }

    pauseAudio();
    restoreSample(dstSmp);
    restoreSample(srcSmp);

    for (i = 0; i < max8Size; ++i)
    {
        x1 = (i >= mix8Size) ? 0 : getSampleValueNr(mixPtr, mixTyp, (mixTyp & 16) ? (i * 2) : i);
        x2 = (i >= dst8Size) ? 0 : getSampleValueNr(dstPtr, dstTyp, (dstTyp & 16) ? (i * 2) : i);

        if (!(mixTyp & 16)) x1 *= 256;
        if (!(dstTyp & 16)) x2 *= 256;

        dSmp = ((x1 * mix_Balance) + (x2 * (100 - mix_Balance))) / 100.0;
        double2int32_round(smp32, dSmp);
        CLAMP16(smp32);

        if (!(dstTyp & 16))
            smp32 >>= 8;

        putSampleValueNr(p, dstTyp, (dstTyp & 16) ? (i * 2) : i, (int16_t)(smp32));
    }

    if (dstSmp->pek != NULL)
        free(dstSmp->pek);

    if (currSmp->typ & 16)
        maxLen &= 0xFFFFFFFE;

    dstSmp->pek    = p;
    dstSmp->len    = maxLen;
    dstSmp->typ    = dstTyp;
    dstSmp->relTon = dstRelTone;

    fixSample(srcSmp);
    fixSample(dstSmp);
    resumeAudio();

    updateNewSample();
    setSongModifiedFlag();
}

void sbSetMixBalancePos(int32_t pos)
{
    if (pos != mix_Balance)
        mix_Balance = (int8_t)(pos);
}

void pbMixBalanceDown(void)
{
    if (mix_Balance > 0)
        mix_Balance--;
}

void pbMixBalanceUp(void)
{
    if (mix_Balance < 100)
        mix_Balance++;
}

void drawMixSampleBox(void)
{
    const int16_t x = 192;
    const int16_t y = 240;
    const int16_t w = 248;
    const int16_t h = 38;
    uint16_t i;
    sysReq_t *sysReq;

    /* main fill */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* outer border */
    vLine(x,         y,         h - 1, PAL_BUTTON1);
    hLine(x + 1,     y,         w - 2, PAL_BUTTON1);
    vLine(x + w - 1, y,         h,     PAL_BUTTON2);
    hLine(x,         y + h - 1, w - 1, PAL_BUTTON2);

    /* inner border */
    vLine(x + 2,     y + 2,     h - 5, PAL_BUTTON2);
    hLine(x + 3,     y + 2,     w - 6, PAL_BUTTON2);
    vLine(x + w - 3, y + 2,     h - 4, PAL_BUTTON1);
    hLine(x + 2,     y + h - 3, w - 4, PAL_BUTTON1);

    textOutShadow(198, 246, PAL_FORGRND, PAL_BUTTON2, "Mixing balance");

    MY_ASSERT((mix_Balance >= 0) && (mix_Balance <= 100))

    charOutFast(299 + (0 * 7), 246, PAL_FORGRND, '0' + ((mix_Balance / 100) % 10));
    charOutFast(299 + (1 * 7), 246, PAL_FORGRND, '0' + ((mix_Balance / 10) % 10));
    charOutFast(299 + (2 * 7), 246, PAL_FORGRND, '0' + (mix_Balance % 10));

    /* show/activate push buttons... */
    sysReq = &sysReqs[editor.ui.systemRequestID];
    for (i = 0; i < sysReq->numButtons; ++i)
        showPushButton(sysReq->buttonIDs[i]);

    showScrollBar(SB_MIX_BALANCE);
    setScrollBarPos(SB_MIX_BALANCE, mix_Balance, false);
}

void sampleMixSample(void)
{
    if (editor.curInstr == 0)
        return;

    sysReqQueue(SR_SAMP_MIX_SAMPLE);
}

void sbSetStartVolPos(int32_t pos)
{
    int16_t val;

    val = (int16_t)(pos - 400);
    if (val != vol_StartVol)
    {
             if (ABS(val) < 10)       val =    0;
        else if (ABS(val - 100) < 10) val =  100;
        else if (ABS(val - 200) < 10) val =  200;
        else if (ABS(val - 300) < 10) val =  300;
        else if (ABS(val + 100) < 10) val = -100;
        else if (ABS(val + 200) < 10) val = -200;
        else if (ABS(val + 300) < 10) val = -300;

        vol_StartVol = val;
    }
}

void sbSetEndVolPos(int32_t pos)
{
    int16_t val;

    val = (int16_t)(pos - 400);
    if (val != vol_EndVol)
    {
             if (ABS(val) < 10)       val =    0;
        else if (ABS(val - 100) < 10) val =  100;
        else if (ABS(val - 200) < 10) val =  200;
        else if (ABS(val - 300) < 10) val =  300;
        else if (ABS(val + 100) < 10) val = -100;
        else if (ABS(val + 200) < 10) val = -200;
        else if (ABS(val + 300) < 10) val = -300;

        vol_EndVol = val;
    }
}

void pbSampStartVolDown(void)
{
    if (vol_StartVol > -400)
        vol_StartVol--;
}

void pbSampStartVolUp(void)
{
    if (vol_StartVol < 400)
        vol_StartVol++;
}

void pbSampEndVolDown(void)
{
    if (vol_EndVol > -400)
        vol_EndVol--;
}

void pbSampEndVolUp(void)
{
    if (vol_EndVol < 400)
        vol_EndVol++;
}

void sampleChangeVolume(void) /* called from sys. req */
{
    int8_t *ptr8;
    int16_t *ptr16;
    int32_t smp;
    int32_t x1, x2, len, i;
    double dSmp;

    hideSystemRequest();

    if ((currSmp == NULL) || (currSmp->pek == NULL) || (currSmp->len == 0) || ((vol_StartVol == 100) && (vol_EndVol == 100)))
        return;

    if (smpEd_Rx1 < smpEd_Rx2)
    {
        x1 = smpEd_Rx1;
        x2 = smpEd_Rx2;

        if (x1 < 0)
            x1 = 0;

        if (x2 > currSmp->len)
            x2 = currSmp->len;

        if (x2 <= x1)
            return;
    }
    else
    {
        x1 = 0;
        x2 = currSmp->len;
    }

    if (currSmp->typ & 16)
    {
        x1 /= 2;
        x2 /= 2;
    }

    len = x2 - x1;

    /* the rounding here is slightly wrong for a PCM waveform, but no one will notice... */

    restoreSample(currSmp);
    if (currSmp->typ & 16)
    {
        ptr16 = (int16_t *)(currSmp->pek);
        for (i = x1; i < x2; ++i)
        {
            dSmp = (ptr16[i] * (vol_StartVol + (((vol_EndVol - vol_StartVol) * (i - x1)) / (double)(len)))) / 100.0;
            double2int32_round(smp, dSmp);
            CLAMP16(smp);
            ptr16[i] = (int16_t)(smp);
        }
    }
    else
    {
        ptr8 = currSmp->pek;
        for (i = x1; i < x2; ++i)
        {
            dSmp = (ptr8[i] * (vol_StartVol + (((vol_EndVol - vol_StartVol) * (i - x1)) / (double)(len)))) / 100.0;
            double2int32_round(smp, dSmp);
            CLAMP8(smp);
            ptr8[i] = (int8_t)(smp);
        }
    }
    fixSample(currSmp);

    writeSample(true);
    setSongModifiedFlag();
}

void sampleGetMaxVolume(void) /* called from sys. req */
{
    int8_t *ptr8;
    int16_t *ptr16, vol;
    int32_t absSmp, x1, x2, len, i, maxAmp;

    if ((currSmp == NULL) || (currSmp->pek == NULL) || (currSmp->len == 0))
    {
        vol_StartVol = 0;
        vol_EndVol   = 0;

        setScrollBarPos(SB_SAMPVOL_START, 400 + vol_StartVol, true);
        setScrollBarPos(SB_SAMPVOL_END, 400 + vol_EndVol, true);

        return;
    }

    if (smpEd_Rx1 < smpEd_Rx2)
    {
        x1 = smpEd_Rx1;
        x2 = smpEd_Rx2;

        if (x1 < 0)
            x1 = 0;

        if (x2 > currSmp->len)
            x2 = currSmp->len;

        if (x2 <= x1)
            return;
    }
    else
    {
        x1 = 0;
        x2 = currSmp->len;
    }

    if (currSmp->typ & 16)
    {
        x1 /= 2;
        x2 /= 2;
    }

    len = x2 - x1;

    restoreSample(currSmp);

    maxAmp = 0;
    if (currSmp->typ & 16)
    {
        ptr16 = (int16_t *)(&currSmp->pek[x1 * 2]);
        for (i = 0; i < len; ++i)
        {
            absSmp = ABS(ptr16[i]);
            if (absSmp > maxAmp)
                maxAmp = absSmp;
        }
    }
    else
    {
        ptr8 = &currSmp->pek[x1];
        for (i = 0; i < len; ++i)
        {
            absSmp = ABS(ptr8[i]);
            if (absSmp > maxAmp)
                maxAmp = absSmp;
        }
    }

    fixSample(currSmp);

    if (maxAmp <= 0)
    {
        vol_StartVol = 0;
        vol_EndVol   = 0;
    }
    else
    {
        vol = (int16_t)(((100 * ((currSmp->typ & 16) ? 32768 : 128)) / maxAmp));
        vol = CLAMP(vol, 100, 400);

        vol_StartVol = vol;
        vol_EndVol   = vol;
    }

    if ((400 + vol_StartVol) != scrollBars[SB_SAMPVOL_START].pos)
        setScrollBarPos(SB_SAMPVOL_START, 400 + vol_StartVol, true);

    if ((400 + vol_EndVol) != scrollBars[SB_SAMPVOL_END].pos)
        setScrollBarPos(SB_SAMPVOL_END, 400 + vol_EndVol, true);
}

void drawSampleVolumeBox(void)
{
    char sign;
    const int16_t x = 166;
    const int16_t y = 230;
    const int16_t w = 301;
    const int16_t h = 52;
    uint16_t i, val;
    sysReq_t *sysReq;

    /* main fill */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* outer border */
    vLine(x,         y,         h - 1, PAL_BUTTON1);
    hLine(x + 1,     y,         w - 2, PAL_BUTTON1);
    vLine(x + w - 1, y,         h,     PAL_BUTTON2);
    hLine(x,         y + h - 1, w - 1, PAL_BUTTON2);

    /* inner border */
    vLine(x + 2,     y + 2,     h - 5, PAL_BUTTON2);
    hLine(x + 3,     y + 2,     w - 6, PAL_BUTTON2);
    vLine(x + w - 3, y + 2,     h - 4, PAL_BUTTON1);
    hLine(x + 2,     y + h - 3, w - 4, PAL_BUTTON1);

    textOutShadow(172, 236, PAL_FORGRND, PAL_BUTTON2, "Start volume");
    textOutShadow(172, 249, PAL_FORGRND, PAL_BUTTON2, "End volume");
    charOutShadow(282, 236, PAL_FORGRND, PAL_BUTTON2, '%');
    charOutShadow(282, 250, PAL_FORGRND, PAL_BUTTON2, '%');

    if (vol_StartVol == 0)
        sign = ' ';
    else if (vol_StartVol < 0)
        sign = '-';
    else
        sign = '+';

    val = ABS(vol_StartVol);
    if (val > 99)
    {
        charOutFast(253, 236, PAL_FORGRND, sign);
        charOutFast(260, 236, PAL_FORGRND, '0' + ((val / 100) % 10));
        charOutFast(267, 236, PAL_FORGRND, '0' + ((val / 10) % 10));
        charOutFast(274, 236, PAL_FORGRND, '0' + (val % 10));
    }
    else if (val > 9)
    {
        charOutFast(260, 236, PAL_FORGRND, sign);
        charOutFast(267, 236, PAL_FORGRND, '0' + ((val / 10) % 10));
        charOutFast(274, 236, PAL_FORGRND, '0' + (val % 10));
    }
    else
    {
        charOutFast(267, 236, PAL_FORGRND, sign);
        charOutFast(274, 236, PAL_FORGRND, '0' + (val % 10));
    }

    if (vol_EndVol == 0)
        sign = ' ';
    else if (vol_EndVol < 0)
        sign = '-';
    else
        sign = '+';

    val = ABS(vol_EndVol);
    if (val > 99)
    {
        charOutFast(253, 250, PAL_FORGRND, sign);
        charOutFast(260, 250, PAL_FORGRND, '0' + ((val / 100) % 10));
        charOutFast(267, 250, PAL_FORGRND, '0' + ((val / 10) % 10));
        charOutFast(274, 250, PAL_FORGRND, '0' + (val % 10));
    }
    else if (val > 9)
    {
        charOutFast(260, 250, PAL_FORGRND, sign);
        charOutFast(267, 250, PAL_FORGRND, '0' + ((val / 10) % 10));
        charOutFast(274, 250, PAL_FORGRND, '0' + (val % 10));
    }
    else
    {
        charOutFast(267, 250, PAL_FORGRND, sign);
        charOutFast(274, 250, PAL_FORGRND, '0' + (val % 10));
    }

    /* show/activate push buttons... */
    sysReq = &sysReqs[editor.ui.systemRequestID];
    for (i = 0; i < sysReq->numButtons; ++i)
        showPushButton(sysReq->buttonIDs[i]);

    showScrollBar(SB_SAMPVOL_START);
    showScrollBar(SB_SAMPVOL_END);
    setScrollBarPos(SB_SAMPVOL_START, 400 + vol_StartVol, false);
    setScrollBarPos(SB_SAMPVOL_END,   400 + vol_EndVol,   false);
}

void sampleVolume(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    sysReqQueue(SR_SAMP_VOLUME);
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

    if ((currSmp->len <= 0) || ((currSmp->typ & 3) == 0))
    {
        hideSprite(SPRITE_LEFT_LOOP_PIN);
        hideSprite(SPRITE_RIGHT_LOOP_PIN);
    }
    else
    {
        /* draw sample loop points */

        repS = smpPos2Scr(curSmpRep);
        repE = smpPos2Scr(curSmpRep + curSmpRepL);

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
        hexOut(536, 375, PAL_FORGRND, curSmpRep,  8);
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

void sbSetResampleTones(int32_t pos)
{
    int8_t val;

    val = (int8_t)(pos - 36);
    if (val != smpEd_RelReSmp)
        smpEd_RelReSmp = val;
}

void pbResampleTonesDown(void)
{
    if (smpEd_RelReSmp > -36)
        smpEd_RelReSmp--;
}

void pbResampleTonesUp(void)
{
    if (smpEd_RelReSmp < 36)
        smpEd_RelReSmp++;
}

void resampleSample(void)
{
    int8_t *p1, *p2, *src8, *dst8;
    int16_t *src16, *dst16;
    int32_t oldLen, newLen, mask, i, readOffset, resampleLen, maxReadLen;
    double dLenMul, dReadOffsetMul;

    hideSystemRequest();

    if ((currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    mask = (currSmp->typ & 16) ? 0xFFFFFFFE : 0xFFFFFFFF;

    dLenMul = pow(2.0, smpEd_RelReSmp / 12.0);
    newLen  = (int32_t)(currSmp->len * dLenMul) & mask;

    p2 = (int8_t *)(malloc(newLen + 2));
    if (p2 == NULL)
    {
        sysReqQueue(SR_OOM_ERROR);
        return;
    }

    p1     = currSmp->pek;
    oldLen = currSmp->len;

    pauseAudio();
    restoreSample(currSmp);

    if (newLen > 0)
    {
        dReadOffsetMul = oldLen / (double)(newLen);

        if (currSmp->typ & 16)
        {
            src16 = (int16_t *)(p1);
            dst16 = (int16_t *)(p2);

            resampleLen = newLen / 2;
            maxReadLen  = oldLen / 2;

            for (i = 0; i < resampleLen; ++i)
            {
                readOffset = (int32_t)(i * dReadOffsetMul); /* truncate */

                if (readOffset >= maxReadLen)
                    dst16[i] = 0;
                else
                    dst16[i] = src16[readOffset];
            }
        }
        else
        {
            src8 = p1;
            dst8 = p2;

            for (i = 0; i < newLen; ++i)
            {
                readOffset = (int32_t)(i * dReadOffsetMul); /* truncate */

                if (readOffset >= oldLen)
                    dst8[i] = 0;
                else
                    dst8[i] = src8[readOffset];
            }
        }
    }

    free(p1);

    currSmp->relTon += smpEd_RelReSmp;
    currSmp->relTon = CLAMP(currSmp->relTon, -48, 71);

    currSmp->len  = newLen;
    currSmp->pek  = p2;
    currSmp->repS = (int32_t)(currSmp->repS * dLenMul) & mask;
    currSmp->repL = (int32_t)(currSmp->repL * dLenMul) & mask;

    if (currSmp->repS > currSmp->len)
        currSmp->repS = currSmp->len;

    if ((currSmp->repS + currSmp->repL) > currSmp->len)
        currSmp->repL  = currSmp->len   - currSmp->repS;

    if (currSmp->typ & 16)
    {
        currSmp->len  &= 0xFFFFFFFE;
        currSmp->repS &= 0xFFFFFFFE;
        currSmp->repL &= 0xFFFFFFFE;
    }

    fixSample(currSmp);
    resumeAudio();

    updateNewSample();
    setSongModifiedFlag();
}

void drawResampleBox(void)
{
    char sign;
    const int16_t x = 209;
    const int16_t y = 230;
    const int16_t w = 214;
    const int16_t h = 54;
    uint16_t i, val;
    uint32_t mask;
    double dLenMul;
    sysReq_t *sysReq;

    /* main fill */
    fillRect(x + 1, y + 1, w - 2, h - 2, PAL_BUTTONS);

    /* outer border */
    vLine(x,         y,         h - 1, PAL_BUTTON1);
    hLine(x + 1,     y,         w - 2, PAL_BUTTON1);
    vLine(x + w - 1, y,         h,     PAL_BUTTON2);
    hLine(x,         y + h - 1, w - 1, PAL_BUTTON2);

    /* inner border */
    vLine(x + 2,     y + 2,     h - 5, PAL_BUTTON2);
    hLine(x + 3,     y + 2,     w - 6, PAL_BUTTON2);
    vLine(x + w - 3, y + 2,     h - 4, PAL_BUTTON1);
    hLine(x + 2,     y + h - 3, w - 4, PAL_BUTTON1);

    mask    = (currSmp->typ & 16) ? 0xFFFFFFFE : 0xFFFFFFFF;
    dLenMul = pow(2.0, smpEd_RelReSmp / 12.0);

    textOutShadow(215, 236, PAL_FORGRND, PAL_BUTTON2, "Rel. h.tones");
    textOutShadow(215, 250, PAL_FORGRND, PAL_BUTTON2, "New sample size");
    hexOut(361, 250, PAL_FORGRND, (uint32_t)(currSmp->len * dLenMul) & mask, 8);

     if (smpEd_RelReSmp == 0)
        sign = ' ';
    else if (smpEd_RelReSmp < 0)
        sign = '-';
    else
        sign = '+';

    val = ABS(smpEd_RelReSmp);
    if (val > 9)
    {
        charOutFast(291, 236, PAL_FORGRND, sign);
        charOutFast(298, 236, PAL_FORGRND, '0' + ((val / 10) % 10));
        charOutFast(305, 236, PAL_FORGRND, '0' + (val % 10));
    }
    else
    {
        charOutFast(298, 236, PAL_FORGRND, sign);
        charOutFast(305, 236, PAL_FORGRND, '0' + (val % 10));
    }

    /* show/activate push buttons... */
    sysReq = &sysReqs[editor.ui.systemRequestID];
    for (i = 0; i < sysReq->numButtons; ++i)
        showPushButton(sysReq->buttonIDs[i]);

    showScrollBar(SB_RESAMPLE_HTONES);
    setScrollBarPos(SB_RESAMPLE_HTONES, smpEd_RelReSmp + 36, false);
}

void sampleResample(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    smpEd_RelReSmp = 0;
    sysReqQueue(SR_SAMP_RESAMPLE);
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
            sysReqQueue(SR_OOM_ERROR);
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

        *outMin = SAMPLE_AREA_Y_CENTER - (int8_t)((min16 * SAMPLE_AREA_HEIGHT) / 65536);
        *outMax = SAMPLE_AREA_Y_CENTER - (int8_t)((max16 * SAMPLE_AREA_HEIGHT) / 65536);
    }
    else
    {
        /* 8-bit sample */

        getMinMax8(&currSmp->pek[index], numBytes, &min8, &max8);

        *outMin = SAMPLE_AREA_Y_CENTER - (int8_t)((min8 * SAMPLE_AREA_HEIGHT) / 256);
        *outMax = SAMPLE_AREA_Y_CENTER - (int8_t)((max8 * SAMPLE_AREA_HEIGHT) / 256);
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
        curSmpRep  = 0;
        curSmpRepL = 0;
    }
    else
    {
        curSmpRep  = currSmp->repS;
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

    smpEd_ScrPos   = 0;
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

    charOutFast(7,  369, PAL_FORGRND, noteChar1);
    charOutFast(15, 369, PAL_FORGRND, noteChar2);
    charOutFast(23, 369, PAL_FORGRND, octaChar);

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
    int32_t newViewSize;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (smpEd_Rx1 < smpEd_Rx2)
    {
        newViewSize = smpEd_Rx2 - smpEd_Rx1;

        if (currSmp->typ & 16)
        {
            newViewSize &= 0xFFFFFFFE;
            if (newViewSize < 4)
                newViewSize = 4;
        }
        else if (newViewSize < 2)
        {
            newViewSize = 2;
        }

        smpEd_ViewSize = newViewSize;
        smpEd_ScrPos   = smpEd_Rx1;
    }
}

void rangeAll(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    smpEd_Rx1 = smpEd_ScrPos;
    smpEd_Rx2 = smpEd_ScrPos + smpEd_ViewSize;

    if (currSmp->typ & 16) /* shouldn't be needed, but for safety */
    {
        smpEd_Rx1 &= 0xFFFFFFFE;
        smpEd_Rx2 &= 0xFFFFFFFE;
    }
}

static void zoomSampleDataIn(int32_t step, int16_t x)
{
    int32_t newViewSize32, newScrPos32;
    int64_t newScrPos64;

    if (currSmp->typ & 16)
    {
        if (old_ViewSize <= 4)
            return;
    }
    else
    {
        if (old_ViewSize <= 2)
            return;
    }

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (step < 1)
        step = 1;

    newViewSize32 = old_ViewSize  - (step * 2);
    if (currSmp->typ & 16)
    {
        if (newViewSize32 < 4)
            newViewSize32 = 4;
    }
    else if (newViewSize32 < 2)
    {
        newViewSize32 = 2;
    }

    step += (((x - (SCREEN_W / 2)) * step) / (SCREEN_W / 2));

    newScrPos64 = old_SmpScrPos + step;
    if ((newScrPos64 + newViewSize32) > currSmp->len)
        newScrPos64 = currSmp->len - newViewSize32;

    newScrPos32 = newScrPos64 & 0xFFFFFFFF;

    smpEd_ViewSize = newViewSize32;
    smpEd_ScrPos   = newScrPos32;
}

static void zoomSampleDataOut(int32_t step, int16_t x)
{
    int32_t newScrPos32, newViewSize32;
    int64_t newViewSize64;

    if ((old_ViewSize == currSmp->len) || (editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (step < 1)
        step = 1;

    newViewSize64 = old_ViewSize + (step * 2);
    if (newViewSize64 > currSmp->len)
    {
        newScrPos32   = 0;
        newViewSize32 = currSmp->len;
    }
    else
    {
        step += (((x - (SCREEN_W / 2)) * step) / (SCREEN_W / 2));

        newViewSize32 = newViewSize64 & 0xFFFFFFFF;

        newScrPos32 = old_SmpScrPos - step;
        if (newScrPos32 < 0)
            newScrPos32 = 0;

        if ((newScrPos32 + newViewSize32) > currSmp->len)
            newScrPos32 = currSmp->len - newViewSize32;
    }

    smpEd_ViewSize = newViewSize32;
    smpEd_ScrPos   = newScrPos32;
}

void mouseZoomSampleDataIn(void)
{
    int32_t step;

    step = (int32_t)(round(old_ViewSize / 10.0));
    zoomSampleDataIn(step, mouse.x);
}

void mouseZoomSampleDataOut(void)
{
    int32_t step;

    step = (int32_t)(round(old_ViewSize / 10.0));
    zoomSampleDataOut(step, mouse.x);
}

void zoomSampleDataOut2x(void)
{
    int32_t step;

    step = (int32_t)(round(old_ViewSize / 2.0));
    zoomSampleDataOut(step, SCREEN_W / 2);
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
    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    if (smpEd_Rx1 >= smpEd_Rx2)
    {
        sysReqQueue(SR_NO_RANGE);
        return;
    }

    memset(smpEd_SysReqText, 0, sizeof (smpEd_SysReqText));
    setupTextBoxForSysReq(TB_SAVE_RANGE_FILENAME, smpEd_SysReqText, sizeof (smpEd_SysReqText) - 1, false);
    sysReqQueue(SR_SAMP_SAVERANGE);
}

void saveRange2(void) /* called from sys. req */
{
    UNICHAR *filenameU;

    hideSystemRequest();

    if (smpEd_SysReqText[0] == '\0')
    {
        sysReqQueue(SR_EMPTY_FILENAME);
        return;
    }

    /* test if the very first character has a dot... */
    if (smpEd_SysReqText[0] == '.')
    {
        sysReqQueue(SR_ILLEGAL_FILENAME_DOT);
        return;
    }

    /* test for illegal file name */
    if ((smpEd_SysReqText[0] == '\0') || (strpbrk(smpEd_SysReqText, "\\/:*?\"<>|") != NULL))
    {
        sysReqQueue(SR_ILLEGAL_FILENAME);
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
        sysReqQueue(SR_SAVE_IO_ERROR);
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

            sysReqQueue(SR_OOM_ERROR);
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

            sysReqQueue(SR_OOM_ERROR);
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
            currSmp->typ &= ~3; /* disable loop */
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
        sysReqQueue(SR_CUT_TO_BUF_OOM);
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
        sysReqQueue(SR_OOM_ERROR);
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
            sysReqQueue(SR_OOM_ERROR);
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
        sysReqQueue(SR_OOM_ERROR);
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
        sysReqQueue(SR_XFADE_ERROR_4);
        return;
    }

    /* check if we selected a range */
    if (smpEd_Rx2 == 0)
    {
        sysReqQueue(SR_XFADE_ERROR_3);
        return;
    }

    /* check if we selected a valid range length */
    if ((smpEd_Rx2 - smpEd_Rx1) <= 2)
    {
        sysReqQueue(SR_XFADE_ERROR_2);
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
                sysReqQueue(SR_XFADE_ERROR_1);
                return;
            }

            d1 = y1 - x1;
            if ((x2 - y1) > d1)
                d1 = x2 - y1;

            d2 = y1 - x1;
            d3 = x2 - y1;

            if ((d1 < 1) || (d2 < 1) || (d3 < 1))
            {
                sysReqQueue(SR_XFADE_ERROR_1);
                return;
            }

            if (((y1 - d1) < 0) || ((y1 + d1) >= currSmp->len))
            {
                sysReqQueue(SR_XFADE_ERROR_2);
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
                sysReqQueue(SR_XFADE_ERROR_1);
                return;
            }

            d1 = y1 - x1;
            if ((x2 - y1) > d1)
                d1 = x2 - y1;

            d2 = y1 - x1;
            d3 = x2 - y1;

            if ((d1 < 1) || (d2 < 1) || (d3 < 1))
            {
                sysReqQueue(SR_XFADE_ERROR_1);
                return;
            }

            if (((y1 - d1) < 0) || ((y1 + d1) >= currSmp->len))
            {
                sysReqQueue(SR_XFADE_ERROR_2);
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
            sysReqQueue(SR_XFADE_ERROR_1);
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
            sysReqQueue(SR_XFADE_ERROR_2);
            return;
        }

        d1 = x2 - x1;
        d2 = currSmp->repS - y1;
        d3 = x2 - x1 - d2;

        if (((y1 + (x2 - x1)) <= currSmp->repS) || (d1 == 0) || (d3 == 0) || (d1 > currSmp->repL))
        {
            sysReqQueue(SR_XFADE_ERROR_1);
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
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    radioButtons[RB_SAMPLE_8BIT].state  = RADIOBUTTON_CHECKED;
    radioButtons[RB_SAMPLE_16BIT].state = RADIOBUTTON_UNCHECKED;
    drawRadioButton(RB_SAMPLE_8BIT);
    drawRadioButton(RB_SAMPLE_16BIT);

    sysReqQueue(SR_SAMP_CONV_8BIT);
}

void rbSample16bit(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    radioButtons[RB_SAMPLE_16BIT].state = RADIOBUTTON_CHECKED;
    radioButtons[RB_SAMPLE_8BIT].state  = RADIOBUTTON_UNCHECKED;
    drawRadioButton(RB_SAMPLE_8BIT);
    drawRadioButton(RB_SAMPLE_16BIT);

    sysReqQueue(SR_SAMP_CONV_16BIT);
}

void clearCurSample(void)
{
    hideSystemRequest();

    if (editor.curInstr == 0)
        return;

    lockMixerCallback();

    if (currSmp->pek != NULL)
        free(currSmp->pek);

    memset(currSmp, 0, sizeof (sampleTyp));

    currSmp->pek = NULL;
    currSmp->vol = 64;
    currSmp->pan = 128;

    unlockMixerCallback();

    updateNewSample();
    setSongModifiedFlag();
}

void convSampleTo8BitCancel(void)
{
    hideSystemRequest();

    lockMixerCallback();
    restoreSample(currSmp);

    currSmp->typ &= ~16; /* remove 16-bit flag */

    fixSample(currSmp);
    unlockMixerCallback();

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void convSampleTo16BitCancel(void)
{
    hideSystemRequest();

    lockMixerCallback();
    restoreSample(currSmp);

    currSmp->typ |= 16; /* add 16-bit flag */

    /* make sure stuff is 2-byte aligned for 16-bit mode */
    currSmp->repS &= 0xFFFFFFFE;
    currSmp->repL &= 0xFFFFFFFE;
    currSmp->len  &= 0xFFFFFFFE;

    fixSample(currSmp);
    unlockMixerCallback();

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
    setSongModifiedFlag();
}

void convSampleTo8Bit(void)
{
    int8_t *dst8;
    int16_t *src16;
    int32_t i, newLen;

    hideSystemRequest();

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    newLen = currSmp->len / 2;

    dst8 = (int8_t *)(malloc(newLen + 2));
    if (dst8 == NULL)
    {
        sysReqQueue(SR_OOM_ERROR);
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

    setSongModifiedFlag();

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
}

void convSampleTo16Bit(void)
{
    int8_t *dst8, *src8;
    int16_t *dst16;
    int32_t i, newLen;

    hideSystemRequest();

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    pauseAudio();
    restoreSample(currSmp);

    newLen = currSmp->len * 2;

    dst8 = (int8_t *)(malloc(newLen + 2));
    if (dst8 == NULL)
    {
        sysReqQueue(SR_OOM_ERROR);
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

    setSongModifiedFlag();

    updateSampleEditorSample();
    updateSampleEditor();
    writeSample(true);
}

void sampClear(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    sysReqQueue(SR_SAMP_CLEAR);
}

void sampMin(void)
{
    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    if (!(currSmp->typ & 3) || ((currSmp->repS + currSmp->repL) == currSmp->len) || ((currSmp->repS + currSmp->repL) <= 0))
        sysReqQueue(SR_SAMP_MINIMIZE_NOT_NEEDED);
    else
        sysReqQueue(SR_SAMP_MINIMIZE);
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

    repS = curSmpRep;
    repL = curSmpRepL;

    loopLen = mouse.rightButtonPressed ? 16 : 1;
    for (i = 0; i < loopLen; ++i)
    {
        if (repS < (currSmp->len - lenSub))
            repS += addVal;

        if ((repS + repL) > currSmp->len)
            repL = currSmp->len - repS;
    }

    curSmpRep  = (currSmp->typ & 16) ? (signed)(repS & 0xFFFFFFFE) : repS;
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

    repS = curSmpRep - delta;
    if (repS < 0)
        repS = 0;

    curSmpRep = (currSmp->typ & 16) ? (signed)(repS & 0xFFFFFFFE) : repS;

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
    if ((curSmpRep + repL) > currSmp->len)
        repL = currSmp->len - curSmpRep;

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

void minimizeSample(void)
{
    hideSystemRequest();

    if ((editor.curInstr == 0) || (currSmp->pek == NULL) || (currSmp->len == 0))
        return;

    if ((currSmp->typ & 3) && (currSmp->repL < currSmp->len))
    {
        lockMixerCallback();
        restoreSample(currSmp);

        currSmp->len = currSmp->repS + currSmp->repL;

        currSmp->pek = (int8_t *)(realloc(currSmp->pek, currSmp->len + 2));
        if (currSmp->pek == NULL)
        {
            freeSample(currSmp);
            sysReqQueue(SR_OOM_ERROR);
        }

        fixSample(currSmp);
        unlockMixerCallback();

        updateSampleEditorSample();
        updateSampleEditor();
        setSongModifiedFlag();
    }
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

    newPos = scr2SmpPos(x) - curSmpRep;

    repS = curSmpRep  + newPos;
    repL = curSmpRepL - newPos;

    if (repS < 0)
    {
        repL += repS;
        repS  = 0;
    }

    if (repL < 0)
    {
        repL = 0;
        repS = curSmpRep + curSmpRepL;
    }

    if (currSmp->typ & 16)
    {
        repS &= 0xFFFFFFFE;
        repL &= 0xFFFFFFFE;
    }

    curSmpRep  = repS;
    curSmpRepL = repL;

    fixRepeatGadgets();
    updateLoopsOnMouseUp = true;
}

void setRightLoopPinPos(int32_t x)
{
    int32_t repL;

    if ((editor.curInstr == 0) || (currSmp->pek == NULL))
        return;

    repL = scr2SmpPos(x) - curSmpRep;
    if (repL < 0)
        repL = 0;

    if ((repL + curSmpRep) > currSmp->len)
        repL = currSmp->len - curSmpRep;

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

    if ((editor.curInstr == 0) || (currSmp->len <= 0))
        return;

    mx = mouse.x;
    my = mouse.y;

    if (!mouseButtonHeld)
    {
        lastDrawX = scr2SmpPos(mx);
        if (currSmp->typ & 16)
            lastDrawX /= 2;

        lastDrawY = ((my - 173) * 255) / SAMPLE_AREA_HEIGHT;
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
        vl = ((my - 173) * 255) / SAMPLE_AREA_HEIGHT;
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
                    tvl = 256 * ((vl - lastDrawY) * (rl - p) / (p - lastDrawX) + vl);
                else
                    tvl = 256 * vl;

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

        lockMixerCallback();
        restoreSample(currSmp);

        if ((currSmp->repS != curSmpRep) || (currSmp->repL != curSmpRepL))
            setSongModifiedFlag();

        currSmp->repS = curSmpRep;
        currSmp->repL = curSmpRepL;

        fixSample(currSmp);
        unlockMixerCallback();

        writeSample(true);
    }
}
