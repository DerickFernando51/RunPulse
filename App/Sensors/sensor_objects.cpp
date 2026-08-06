#include "SensorManager.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c3;
extern SPI_HandleTypeDef hspi1;


MAX30102 max30102(&hi2c1);

KX126 kx126(&hspi1);

MAX17048 max17048(&hi2c3);


SensorManager sensors(
    max30102,
    kx126,
    max17048
);
