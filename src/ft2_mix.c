#include <stdio.h>
#include <stdint.h>
#include "ft2_header.h"
#include "ft2_mix.h"
#include "ft2_mix_macros.h"

/*
   --------------------- fixed-point audio channel mixer ---------------------

   This file has separate routines for EVERY possible sampling variation:
   Interpolation, volume ramping, 8-bit, 16-bit, no loop, loop, bidi loop.
   24 mixing routines in total.

   Every voice has a function pointer set to the according mixing routine on
   sample trigger (from replayer, but set in audio thread), using a function
   pointer look-up table.
   All voices are always cleared (thread safe) when changing any of the above
   states from the GUI, so no problem there with deprecated cached function
   pointers.

   Mixing macros can be found in ft2_mix_macros.h.

   Yes, this is a HUGE mess, and I hope you don't need to modify it.
   If it's not broken, don't try to fix it!
*/

/* ----------------------------------------------------------------------- */
/*                          8-BIT MIXING ROUTINES                          */
/* ----------------------------------------------------------------------- */

static void mix8bNoLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO
                INC_POS
                RENDER_8BIT_SMP_MONO
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP
                INC_POS
                RENDER_8BIT_SMP
                INC_POS
            }
        }

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix8bLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO
                INC_POS
                RENDER_8BIT_SMP_MONO
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP
                INC_POS
                RENDER_8BIT_SMP
                INC_POS
            }
        }

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bBidiLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        CDA_BytesLeft -= samplesToMix;

        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO
                    DEC_POS
                    RENDER_8BIT_SMP_MONO
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP
                    DEC_POS
                    RENDER_8BIT_SMP
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO
                    INC_POS
                    RENDER_8BIT_SMP_MONO
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP
                    INC_POS
                    RENDER_8BIT_SMP
                    INC_POS
                }
            }
        }

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bNoLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_LERP
                INC_POS
                RENDER_8BIT_SMP_LERP
                INC_POS
            }
        }

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix8bLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
                RENDER_8BIT_SMP_MONO_LERP
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_LERP
                INC_POS
                RENDER_8BIT_SMP_LERP
                INC_POS
            }
        }

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bBidiLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        CDA_BytesLeft -= samplesToMix;

        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO_LERP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO_LERP
                    INC_POS
                    RENDER_8BIT_SMP_MONO_LERP
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_LERP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_LERP
                    INC_POS
                    RENDER_8BIT_SMP_LERP
                    INC_POS
                }
            }
        }

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampNoLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampBidiLoop(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_8BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_8BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
        }
        SET_VOL_BACK

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampNoLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_8BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix8bRampBidiLoopLerp(voice_t *v, uint32_t numSamples)
{
    const int8_t *CDA_LinearAdr;
    uint8_t mixInMono, CDA_SmpEndFlag;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int8_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE8

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_8BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_8BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_8BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_8BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_8BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_8BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
        }
        SET_VOL_BACK

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}



/* ----------------------------------------------------------------------- */
/*                          16-BIT MIXING ROUTINES                         */
/* ----------------------------------------------------------------------- */

static void mix16bNoLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO
                INC_POS
                RENDER_16BIT_SMP_MONO
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP
                INC_POS
                RENDER_16BIT_SMP
                INC_POS
            }
        }

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix16bLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO
                INC_POS
                RENDER_16BIT_SMP_MONO
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP
                INC_POS
                RENDER_16BIT_SMP
                INC_POS
            }
        }

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bBidiLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        CDA_BytesLeft -= samplesToMix;

        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO
                    DEC_POS
                    RENDER_16BIT_SMP_MONO
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP
                    DEC_POS
                    RENDER_16BIT_SMP
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO
                    INC_POS
                    RENDER_16BIT_SMP_MONO
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP
                    INC_POS
                    RENDER_16BIT_SMP
                    INC_POS
                }
            }
        }

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bNoLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_LERP
                INC_POS
                RENDER_16BIT_SMP_LERP
                INC_POS
            }
        }

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix16bLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol| CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        CDA_BytesLeft -= samplesToMix;

        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
                RENDER_16BIT_SMP_MONO_LERP
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_LERP
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_LERP
                INC_POS
                RENDER_16BIT_SMP_LERP
                INC_POS
            }
        }

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bBidiLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    GET_VOL

    if ((CDA_LVol | CDA_RVol) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        CDA_BytesLeft -= samplesToMix;

        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO_LERP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO_LERP
                    INC_POS
                    RENDER_16BIT_SMP_MONO_LERP
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_LERP
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_LERP
                    INC_POS
                    RENDER_16BIT_SMP_LERP
                    INC_POS
                }
            }
        }

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampNoLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_MONO
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampBidiLoop(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_16BIT_SMP_MONO
                    VOLUME_RAMPING
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_16BIT_SMP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
        }
        SET_VOL_BACK

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampNoLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t  CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_NO_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        HANDLE_SAMPLE_END
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (mixInMono)
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_MONO_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        else
        {
            if (samplesToMix & 1)
            {
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
            samplesToMix >>= 1;
            for (i = 0; i < samplesToMix; ++i)
            {
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
                RENDER_16BIT_SMP_LERP
                VOLUME_RAMPING
                INC_POS
            }
        }
        SET_VOL_BACK

        WRAP_LOOP
    }

    SET_BACK_MIXER_POS
}

static void mix16bRampBidiLoopLerp(voice_t *v, uint32_t numSamples)
{
    uint8_t mixInMono, CDA_SmpEndFlag;
    const int16_t *CDA_LinearAdr;
    int32_t realPos, sample, sample2, sample3, *audioMixL, *audioMixR, CDA_BytesLeft, CDA_LVolIP, CDA_RVolIP;
    register const int16_t *smpPtr;
    register int32_t CDA_LVol, CDA_RVol;
    register uint32_t pos, delta;
    uint32_t i, samplesToMix;

    if ((v->SLVol1 | v->SRVol1 | v->SLVol2 | v->SRVol2) == 0)
    {
        VOL0_OPTIMIZATION_BIDI_LOOP
        return;
    }

    GET_MIXER_VARS_RAMP
    SET_BASE16

    CDA_BytesLeft = numSamples;
    while (CDA_BytesLeft > 0)
    {
        LIMIT_MIX_NUM_BIDI_LOOP
        LIMIT_MIX_NUM_RAMP
        CDA_BytesLeft -= samplesToMix;

        GET_VOL
        if (v->backwards)
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_16BIT_SMP_MONO_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                    RENDER_16BIT_SMP_LERP_BACKWARDS
                    VOLUME_RAMPING
                    DEC_POS
                }
            }
        }
        else
        {
            if (mixInMono)
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_16BIT_SMP_MONO_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
            else
            {
                if (samplesToMix & 1)
                {
                    RENDER_16BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
                samplesToMix >>= 1;
                for (i = 0; i < samplesToMix; ++i)
                {
                    RENDER_16BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                    RENDER_16BIT_SMP_LERP
                    VOLUME_RAMPING
                    INC_POS
                }
            }
        }
        SET_VOL_BACK

        WRAP_BIDI_LOOP
    }

    SET_BACK_MIXER_POS
}

/* ----------------------------------------------------------------------- */

const mixRoutine mixRoutineTable[24] =
{
    (mixRoutine)(mix8bNoLoop),
    (mixRoutine)(mix8bLoop),
    (mixRoutine)(mix8bBidiLoop),
    (mixRoutine)(mix8bNoLoopLerp),
    (mixRoutine)(mix8bLoopLerp),
    (mixRoutine)(mix8bBidiLoopLerp),
    (mixRoutine)(mix8bRampNoLoop),
    (mixRoutine)(mix8bRampLoop),
    (mixRoutine)(mix8bRampBidiLoop),
    (mixRoutine)(mix8bRampNoLoopLerp),
    (mixRoutine)(mix8bRampLoopLerp),
    (mixRoutine)(mix8bRampBidiLoopLerp),
    (mixRoutine)(mix16bNoLoop),
    (mixRoutine)(mix16bLoop),
    (mixRoutine)(mix16bBidiLoop),
    (mixRoutine)(mix16bNoLoopLerp),
    (mixRoutine)(mix16bLoopLerp),
    (mixRoutine)(mix16bBidiLoopLerp),
    (mixRoutine)(mix16bRampNoLoop),
    (mixRoutine)(mix16bRampLoop),
    (mixRoutine)(mix16bRampBidiLoop),
    (mixRoutine)(mix16bRampNoLoopLerp),
    (mixRoutine)(mix16bRampLoopLerp),
    (mixRoutine)(mix16bRampBidiLoopLerp)
};
