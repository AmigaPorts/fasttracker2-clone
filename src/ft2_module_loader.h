#ifndef __FT2_MODULE_LOADER_H
#define __FT2_MODULE_LOADER_H

#include <stdint.h>
#include "ft2_unicode.h"

void loadMusic(UNICHAR *filenameU);
int8_t loadMusicUnthreaded(UNICHAR *filenameU); /* for development testing */
int8_t handleModuleLoadFromArg(int argc, char **argv);
void loadDroppedFile(char *fullPathUTF8, uint8_t songModifiedCheck);
void loadDroppedFile2(void); /* called from sys. req. */

/* called from sys. req. if song was unsaved and you selected "No" to load */
void cancelLoadDroppedFile(void);

void handleLoadMusicEvents(void);
void clearUnusedChannels(tonTyp *p, int16_t pattLen, uint8_t antChn);
void unpackPatt(uint8_t *dst, uint16_t inn, uint16_t len, uint8_t antChn);

#endif
