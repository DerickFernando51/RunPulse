#include "max30102.h"

static I2C_HandleTypeDef *_hi2c;  // store the handle internally

// ── Private primitives ────────────────────────────────────

static MAX30102_Status_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    if (HAL_I2C_Master_Transmit(_hi2c, MAX30102_ADDR,
                                 buf, 2, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    return MAX30102_OK;
}

static MAX30102_Status_t read_reg(uint8_t reg, uint8_t *out)
{
    if (HAL_I2C_Master_Transmit(_hi2c, MAX30102_ADDR,
                                 &reg, 1, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    if (HAL_I2C_Master_Receive(_hi2c, MAX30102_ADDR,
                                out, 1, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    return MAX30102_OK;
}

static MAX30102_Status_t read_burst(uint8_t reg,
                                     uint8_t *buf,
                                     uint8_t  len)
{
    if (HAL_I2C_Master_Transmit(_hi2c, MAX30102_ADDR,
                                 &reg, 1, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    if (HAL_I2C_Master_Receive(_hi2c, MAX30102_ADDR,
                                buf, len, 10) != HAL_OK)
        return MAX30102_ERR_I2C;
    return MAX30102_OK;
}

MAX30102_Status_t MAX30102_Init(I2C_HandleTypeDef *hi2c)
{
    _hi2c = hi2c;
    MAX30102_Status_t s;

    s = write_reg(REG_MODE_CONFIG,   0x40); // reset
    if (s != MAX30102_OK) return s;
    HAL_Delay(100);

    s = write_reg(REG_FIFO_CONFIG,   0x4F); // avg=4, rollover on
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_MODE_CONFIG,   0x03); // SpO2 mode
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_SPO2_CONFIG,   0x27); // 100Hz, 18-bit
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_LED1_PA,       0x24); // RED current
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_LED2_PA,       0x24); // IR current
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_WR_PTR,  0x00);
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_OVF_CTR, 0x00);
    if (s != MAX30102_OK) return s;
    s = write_reg(REG_FIFO_RD_PTR,  0x00);
    return s;
}

MAX30102_Status_t MAX30102_ReadSample(MAX30102_Sample_t *sample)
{
    uint8_t raw[6];
    MAX30102_Status_t s = read_burst(REG_FIFO_DATA, raw, 6);
    if (s != MAX30102_OK) return s;

    sample->red = ((uint32_t)(raw[0] & 0x03) << 16)
                | ((uint32_t) raw[1]          <<  8)
                |  (uint32_t) raw[2];

    sample->ir  = ((uint32_t)(raw[3] & 0x03) << 16)
                | ((uint32_t) raw[4]          <<  8)
                |  (uint32_t) raw[5];

    return MAX30102_OK;
}

bool MAX30102_FingerPresent(MAX30102_Sample_t *sample)
{
    return sample->ir > 50000;
}
