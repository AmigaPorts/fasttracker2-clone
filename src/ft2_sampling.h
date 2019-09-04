#ifndef __FT2_SAMPLING_H
#define __FT2_SAMPLING_H

void askToSample(void);
void srStartSampling(void); /*called from sys. request */
void stopSampling(void);
void handleSamplingUpdates(void);

#endif
