#ifndef PPG_DSP_H
#define PPG_DSP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint16_t heart_rate;      // BPM
    uint8_t  spo2;            // %
    uint8_t  signal_quality;  // 0-100
    uint8_t  valid;           // result valid flag

} PPG_Result_t;

void PPG_Init(void);

void PPG_SetFingerPresent(bool present);

void PPG_PushSample(
    uint32_t red_val,
    uint32_t ir_val
);

void PPG_Process(void);

PPG_Result_t PPG_GetResult(void);

#ifdef __cplusplus
}
#endif

#endif
