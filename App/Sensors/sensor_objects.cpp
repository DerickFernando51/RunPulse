#include "SensorManager.h"
#include "MAX30102.h"
#include "KX126.h"
#include "MAX17048.h"

#include "main.h"

#include "MAX30102_Callback.h"

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

void Sensors_Init()
{
    MAX30102_RegisterInstance(&max30102);
}
