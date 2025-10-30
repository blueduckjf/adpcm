#include "adpcm.h"

//Step size table (49 entries for 8-bit range)
static const uint16_t stepTable[49] = 
{
    1, 1, 1, 1, 2, 2, 2, 3, 3, 4,
    4, 5, 5, 6, 7, 8, 9, 10, 11, 13,
    14, 16, 18, 20, 22, 25, 28, 31, 35, 39,
    44, 49, 55, 61, 68, 76, 85, 95, 106, 118,
    132, 147, 164, 183, 204, 228, 254, 283, 315
};

#define STEP_TABLE_MAX_INDEX    (sizeof(stepTable) - 1)

//Index adjustment table
static const int8_t indexTable[16] = 
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

static void Adpcm_ClampState(adpcmState_t* state, uint8_t nibble)
{
    if (state == NULL) return;

    //Clamp to 8 bits
    if (state->predictor > UINT8_MAX) state->predictor = UINT8_MAX;
    if (state->predictor < 0) state->predictor = 0;

    //Update state index
    state->stepIndex += indexTable[nibble];
    if (state->stepIndex < 0) state->stepIndex = 0;
    if (state->stepIndex > STEP_TABLE_MAX_INDEX) state->stepIndex = STEP_TABLE_MAX_INDEX;
}

void Adpcm_Init(adpcmState_t* state, uint8_t firstSample)
{
    if (state == NULL) return;

    state->predictor = firstSample;
    state->stepIndex = 0;
}

uint8_t Adpcm_EncodeSample(adpcmState_t* state, uint8_t sample)
{
    if (state == NULL) return 0;

    int16_t diff = sample - state->predictor;
    uint8_t nibble = 0;

    //Store sign bit
    if (diff < 0)
    {
        nibble = (1 << 3);
        diff = -diff;
    }

    //Calculate nibble from difference
    uint16_t step = stepTable[state->stepIndex];
    int16_t vpDiff = step >> 3; //Bias

    //TODO: seems ripe for generalization
    for (int8_t i=2; i>=0; --i)
    {
        if (diff >= step)
        {
            nibble |= (1 << i);
            diff -= step;
            vpDiff += step;
        }

        step >>= 1;
    }

    //Update predictor
    if (nibble & (1 << 3))
    {
        state->predictor -= vpDiff;
    }
    else
    {
        state->predictor += vpDiff;
    }

    Adpcm_ClampState(state, nibble);

    return nibble;
}

size_t Adpcm_Encode(const uint8_t* input, uint8_t* output, size_t numSamples)
{
    if ((input == NULL) || (output == NULL) || (numSamples == 0)) return 0;

    adpcmState_t state;
    Adpcm_Init(&state, input[0]);

    //First sample is stored as-is for decoder initialization
    output[0] = input[0];

    //Encode remaining samples (2 nibbles per byte)
    size_t outIndex = 1;
    for (size_t i=outIndex; i<numSamples; ++i)
    {
        uint8_t nibble = Adpcm_EncodeSample(&state, input[i]);

        if (i & 0x01)
        {
            output[outIndex] = nibble;
        }
        else
        {
            output[outIndex] |= (nibble << 4);
            ++outIndex;
        }
    }

    //Handle last nibble if there are an odd number of samples
    if ((numSamples & 0x01) == 0)
    {
        ++outIndex;
    }

    return outIndex;
}

uint8_t Adpcm_DecodeSample(adpcmState_t* state, uint8_t nibble)
{
    uint16_t step = stepTable[state->stepIndex];
    int16_t vpDiff = step >> 3; //Bias

    //Calculate difference

    //TODO: Ripe for generalization
    for (uint8_t i=0; i<3; ++i)
    {
        if (nibble & (1 << (2-i))) vpDiff += (step >> i);
    }

    //Apply sign
    if (nibble & (1<<3))
    {
        state->predictor -= vpDiff;
    }
    else
    {
        state->predictor += vpDiff;
    }

    Adpcm_ClampState(state, nibble);

    return (uint8_t)state->predictor;
}

size_t Adpcm_Decode(const uint8_t* input, uint8_t* output, size_t inputSize)
{
    if ((input == NULL) || (output == NULL) || (inputSize == 0)) return 0;

    //Calculate number of output samples from input size
    size_t numSamples = 1 + (inputSize - 1) * 2;

    adpcmState_t state;
    Adpcm_Init(&state, input[0]);

    //First sample is stored as-is
    output[0] = input[0];

    //Decode remaining samples
    size_t inIndex = 1;
    size_t outSamples = 1;
    for (size_t i=inIndex; i<numSamples; ++i)
    {
        uint8_t nibble;

        if (i & 0x01)
        {
            nibble = input[inIndex] & 0x0F;
        }
        else
        {
            nibble = (input[inIndex] >> 4) & 0x0F;
            ++inIndex;
        }

        output[i] = Adpcm_DecodeSample(&state, nibble);
    }

    return numSamples;
}