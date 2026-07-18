#include "max30102.h"

static I2C_HandleTypeDef *_hi2c;

static volatile MAX30102_DmaState_t dma_state = MAX30102_IDLE;
static uint8_t dma_raw[6];

static MAX30102_Status_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    if (HAL_I2C_Master_Transmit(_hi2c, MAX30102_ADDR,
                                 buf, 2, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    return MAX30102_OK;
}

MAX30102_Status_t MAX30102_Init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
    dma_state = MAX30102_IDLE;

    MAX30102_Status_t s;

    s = write_reg(REG_MODE_CONFIG,  0x40); // reset
    if (s != MAX30102_OK) return s;
    HAL_Delay(100);

    s = write_reg(REG_FIFO_CONFIG,  0x4F); // avg=4, rollover on
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_MODE_CONFIG,  0x03); // SpO2 mode
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_SPO2_CONFIG,  0x27); // 100Hz, 18-bit
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_LED1_PA,      0x24); // RED current
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_LED2_PA,      0x24); // IR current
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_WR_PTR,  0x00);
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_OVF_CTR, 0x00);
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_RD_PTR,  0x00);
    return s;
}

// ── Non-blocking DMA read cycle ─────────────────────────────

MAX30102_Status_t MAX30102_StartReadDMA(void)
{
    if (dma_state == MAX30102_BUSY)
        return MAX30102_ERR_BUSY;   // previous read still in flight

    dma_state = MAX30102_BUSY;

    if (HAL_I2C_Mem_Read_DMA(_hi2c, MAX30102_ADDR, REG_FIFO_DATA,
                              I2C_MEMADD_SIZE_8BIT, dma_raw, 6) != HAL_OK)
    {
        dma_state = MAX30102_IDLE;
        return MAX30102_ERR_I2C;
    }
    return MAX30102_OK;
}

MAX30102_DmaState_t MAX30102_GetDmaState(void)
{
    return dma_state;
}

MAX30102_Status_t MAX30102_GetLastSample(MAX30102_Sample_t *sample)
{
    if (dma_state != MAX30102_DATA_READY)
        return MAX30102_ERR_INVALID;

    sample->red = ((uint32_t)(dma_raw[0] & 0x03) << 16)
                | ((uint32_t) dma_raw[1]          <<  8)
                |  (uint32_t) dma_raw[2];

    sample->ir  = ((uint32_t)(dma_raw[3] & 0x03) << 16)
                | ((uint32_t) dma_raw[4]          <<  8)
                |  (uint32_t) dma_raw[5];

    dma_state = MAX30102_IDLE;
    return MAX30102_OK;
}

bool MAX30102_FingerPresent(MAX30102_Sample_t *sample)
{
    return sample->ir > 50000;
}

void MAX30102_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (_hi2c != NULL && hi2c->Instance == _hi2c->Instance)
    {
        dma_state = MAX30102_DATA_READY;
    }
}

