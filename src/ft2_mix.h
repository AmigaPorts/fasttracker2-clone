#ifndef __FT2_MIX_H
#define __FT2_MIX_H

#include <stdint.h>
#include "ft2_audio.h"

/* used for an optimization technique (32 is tested to be a good setting) */
#define MIN_MIX_THRESHOLD 32

typedef void (*mixRoutine)(void *, int32_t);

extern const mixRoutine mixRoutineTable[24]; /* ft2_mix.c */

#endif
