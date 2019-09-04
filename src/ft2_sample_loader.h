#ifndef __FT2_SAMPLE_LOADER_H
#define __FT2_SAMPLE_LOADER_H

#include <stdint.h>
#include "ft2_unicode.h"

int8_t loadSample(UNICHAR *filenameU, uint8_t sampleSlot, uint8_t loadAsInstrFlag);
int8_t fileIsInstrument(char *fullPath);
int8_t fileIsSample(char *fullPath);
void removeSampleIsLoadingFlag(void);

#endif
