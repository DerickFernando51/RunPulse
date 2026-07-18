#ifndef MAX30102_H
#define MAX30102_H

#include "stm32wbxx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// ── Register map ──────────────────────────────────────────
#define MAX30102_ADDR           (0x57 << 1)

#define REG_INTR_STATUS_1       0x00
#define REG_FIFO_WR_PTR         0x04
#define REG_FIFO_OVF_CTR        0x05
#define REG_FIFO_RD_PTR         0x06
#define REG_FIFO_DATA           0x07
#define REG_FIFO_CONFIG         0x08
#define REG_MODE_CONFIG         0x09
#define REG_SPO2_CONFIG         0x0A
#define REG_LED1_PA             0x0C
#define REG_LED2_PA             0x0D

// ── Return codes ──────────────────────────────────────────
typedef enum {
    MAX30102_OK    = 0,
    MAX30102_ERR_I2C,
    MAX30102_ERR_NO_FINGER,
    MAX30102_ERR_INVALID,
    MAX30102_ERR_BUSY
} MAX30102_Status_t;

// ── Data struct ───────────────────────────────────────────
typedef struct {
    uint32_t red;
    uint32_t ir;
} MAX30102_Sample_t;

// ── DMA state machine ─────────────────────────────────────
typedef enum {
    MAX30102_IDLE = 0,
    MAX30102_BUSY,
    MAX30102_DATA_READY
} MAX30102_DmaState_t;


MAX30102_Status_t MAX30102_Init(I2C_HandleTypeDef *hi2c);


MAX30102_Status_t   MAX30102_StartReadDMA(void);
MAX30102_DmaState_t MAX30102_GetDmaState(void);
MAX30102_Status_t   MAX30102_GetLastSample(MAX30102_Sample_t *sample);


bool MAX30102_FingerPresent(MAX30102_Sample_t *sample);


void MAX30102_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c);

#endif
