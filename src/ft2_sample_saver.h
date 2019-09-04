#ifndef __FT2_SAMPLE_SAVER_H
#define __FT2_SAMPLE_SAVER_H

#include <stdint.h>
#include "ft2_unicode.h"

enum
{
    SAVE_NORMAL = 0,
    SAVE_RANGE  = 1
};

void saveSample(UNICHAR *filenameU, uint8_t saveRangeFlag);

#endif
