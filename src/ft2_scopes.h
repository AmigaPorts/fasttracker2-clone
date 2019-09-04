#ifndef __FT2_SCOPES_H
#define __FT2_SCOPES_H

#include <stdint.h>
#include "ft2_header.h"

int32_t getSamplePosition(uint8_t ch);
void stopAllScopes(void);
void setScopeRate(uint8_t ch, uint32_t rate);
void setScopeVolume(uint8_t ch, uint16_t vol);
void latchScope(uint8_t ch, int8_t *pek, int32_t len, int32_t repS, int32_t repL, uint8_t typ, int32_t playOffset);
void unmuteAllChansOnMusicLoad(void);
int8_t testScopesMouseDown(void);
void drawScopes(void);
void drawScopeFramework(void);
uint8_t initScopes(void);

#endif
