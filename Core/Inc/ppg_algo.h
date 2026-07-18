#ifndef PPG_ALGO_H
#define PPG_ALGO_H

#include <stdint.h>

typedef struct {
    uint16_t heart_rate;     // bpm
    uint8_t  spo2;           // %
    uint8_t  signal_quality; // 0-100
    uint8_t  valid;          // 1 once a full window has been processed
} PPG_Result_t;

void PPG_Init(void);
void PPG_PushSample(uint32_t red_val, uint32_t ir_val);
PPG_Result_t PPG_GetLastResult(void);

#endif
