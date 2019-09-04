#ifndef __FT2_SAMPLE_ED_H
#define __FT2_SAMPLE_ED_H

#include <stdint.h>
#include "ft2_replayer.h"

#define SAMPLE_AREA_HEIGHT   154
#define SAMPLE_AREA_WIDTH    632
#define SAMPLE_AREA_Y_CENTER 250

void fixSample(sampleTyp *s); /* adds wrapped sample after loop/end (for branchless linear interpolation) */
void restoreSample(sampleTyp *s); /* reverts wrapped sample after loop/end (for branchless linear interpolation) */

void cbEchoAddMemory(void);
void sbSetEchoNumPos(int32_t pos);
void sbSetEchoDistPos(int32_t pos);
void sbSetEchoFadeoutPos(int32_t pos);
void pbEchoNumDown(void);
void pbEchoNumUp(void);
void pbEchoDistDown(void);
void pbEchoDistUp(void);
void pbEchoFadeoutDown(void);
void pbEchoFadeoutUp(void);

void sbSetMixBalancePos(int32_t pos);
void pbMixBalanceDown(void);
void pbMixBalanceUp(void);
void clearCopyBuffer(void);

void sbSetResampleTones(int32_t pos);
void pbResampleTonesDown(void);
void pbResampleTonesUp(void);
void resampleSample(void);
void sampleResample(void);
void drawResampleBox(void);

uint32_t getSampleMiddleCRate(sampleTyp *s);
int32_t getSampleRangeStart(void);
int32_t getSampleRangeEnd(void);
int32_t getSampleRangeLength(void);

void copySmp(void); /* dstSmp = srcSmp */
void xchgSmp(void); /* dstSmp <-> srcSmp */

/* callbacks */
void scrollSampleDataLeft(void);
void scrollSampleDataRight(void);
void scrollSampleData(int32_t pos);
void sampPlayNoteUp(void);
void sampPlayNoteDown(void);
void sampPlayWave(void);
void sampPlayRange(void);
void sampPlayDisplay(void);
void showRange(void);
void rangeAll(void);
void mouseZoomSampleDataIn(void);
void mouseZoomSampleDataOut(void);
void zoomSampleDataOut2x(void);
void showAll(void);
void saveRange(void);
void sampCut(void);
void sampCopy(void);
void sampPaste(void);
void sampCrop(void);
void sampVolume(void);
void sampXFade(void);
void rbSampleNoLoop(void);
void rbSampleForwardLoop(void);
void rbSamplePingpongLoop(void);
void rbSample8bit(void);
void rbSample16bit(void);
void sampClear(void);
void sampMin(void);
void sampRepeatUp(void);
void sampRepeatDown(void);
void sampReplenUp(void);
void sampReplenDown(void);

void sbSetStartVolPos(int32_t pos);
void sbSetEndVolPos(int32_t pos);
void pbSampStartVolDown(void);
void pbSampStartVolUp(void);
void pbSampEndVolDown(void);
void pbSampEndVolUp(void);

/* system request callbacks */
void clearCurSample(void);
void convSampleTo8BitCancel(void);
void convSampleTo16BitCancel(void);
void convSampleTo8Bit(void);
void convSampleTo16Bit(void);
void minimizeSample(void);
void sampleChangeVolume(void);
void sampleGetMaxVolume(void);
void mixSample(void);
void drawMixSampleBox(void);
void sampleMixSample(void);
void drawEchoBox(void);
void createEcho(void);
void sampleEcho(void);
void saveRange2(void);
/* ----------------------------------------- */

void writeSample(uint8_t forceSmpRedraw);
void clearRange(void);
void setSampleRange(int32_t start, int32_t end);
void handleSampleDataMouseDown(int8_t mouseButtonHeld);
void updateSampleEditorSample(void);
void updateSampleEditor(void);
void hideSampleEditor(void);
void exitSampleEditor(void);
void showSampleEditor(void);
void drawSampleVolumeBox(void);
void sampleVolume(void);
void handleSamplerRedrawing(void);
void toggleSampleEditor(void);

void toggleSampleEditorExt(void);
void showSampleEditorExt(void);
void hideSampleEditorExt(void);
void drawSampleEditorExt(void);

void handleSampleEditorExtRedrawing(void);

void sampleBackwards(void);
void sampleConv(void);
void sampleConvW(void);
void fixDC(void);

void smpEdStop(void);
void testSmpEdMouseUp(void);

#endif
