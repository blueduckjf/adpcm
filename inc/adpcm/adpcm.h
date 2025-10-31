#ifndef ADPCM_H_
#define ADPCM_H_

#include <inttypes.h>
#include <stddef.h>
#include <string.h>

typedef struct
{
    volatile int16_t predictor;
    volatile int8_t stepIndex;
} adpcmState_t;

#ifdef __cplusplus
extern "C" {
#endif

void Adpcm_Init(adpcmState_t* state, uint8_t firstSample);
uint8_t Adpcm_EncodeSample(adpcmState_t* state, uint8_t sample);
size_t Adpcm_Encode(const uint8_t* input, uint8_t* output, size_t numSamples);
uint8_t Adpcm_DecodeSample(adpcmState_t* state, uint8_t nibble);
size_t Adpcm_Decode(const uint8_t* input, uint8_t* output, size_t inputSize);

#ifdef __cplusplus
}
#endif

#endif