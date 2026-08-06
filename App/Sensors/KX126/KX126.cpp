#include "KX126.h"
#include "usbd_cdc_if.h"

KX126::KX126(SPI_HandleTypeDef* hspi)
    :
    hspi_(hspi)
{

}


bool KX126::init()
{
    uint8_t id;


    readRegister(
        KX126_WHO_AM_I_REG,
        &id,
        1
    );


    char msg[40];

    sprintf(msg,
            "KX ID = 0x%02X\r\n",
            id);

    CDC_Transmit_FS(
        (uint8_t*)msg,
        strlen(msg)
    );


    writeRegister(
        KX126_CNTL1,
        0x00
    );

    HAL_Delay(10);


    writeRegister(
        KX126_ODCNTL,
        0x02
    );


    writeRegister(
        KX126_CNTL1,
        0xC0
    );


    HAL_Delay(50);


    return true;
}


bool KX126::readAcceleration(AccelData& data)
{
    uint8_t buffer[6];


    if(!readRegister(
            KX126_XOUT_L,
            buffer,
            6))
    {
        return false;
    }

/*Print raw readings */
//    char msg[80];
//
//    sprintf(msg,
//        "RAW %02X %02X %02X %02X %02X %02X\r\n",
//        buffer[0],
//        buffer[1],
//        buffer[2],
//        buffer[3],
//        buffer[4],
//        buffer[5]
//    );
//
//
//    CDC_Transmit_FS(
//        (uint8_t*)msg,
//        strlen(msg)
//    );


    int16_t x =
        (int16_t)((buffer[1] << 8) | buffer[0]);

    int16_t y =
        (int16_t)((buffer[3] << 8) | buffer[2]);

    int16_t z =
        (int16_t)((buffer[5] << 8) | buffer[4]);


    data.x = x * 0.000061f;
    data.y = y * 0.000061f;
    data.z = z * 0.000061f;


    return true;
}



bool KX126::writeRegister(
        uint8_t reg,
        uint8_t value)
{
    uint8_t tx[2];


    tx[0] = reg & 0x7F;
    tx[1] = value;


    HAL_GPIO_WritePin(
        KX126_CS_PORT,
        KX126_CS_PIN,
        GPIO_PIN_RESET
    );


    HAL_StatusTypeDef status =
        HAL_SPI_Transmit(
            hspi_,
            tx,
            2,
            100
        );


    HAL_GPIO_WritePin(
        KX126_CS_PORT,
        KX126_CS_PIN,
        GPIO_PIN_SET
    );


    return status == HAL_OK;
}


bool KX126::readRegister(
        uint8_t reg,
        uint8_t* data,
        uint16_t length)
{
    uint8_t tx[16]={0};
    uint8_t rx[16]={0};


    tx[0] = reg | KX126_READ;


    HAL_GPIO_WritePin(
        KX126_CS_PORT,
        KX126_CS_PIN,
        GPIO_PIN_RESET
    );


    HAL_StatusTypeDef status =
        HAL_SPI_TransmitReceive(
            hspi_,
            tx,
            rx,
            length + 1,
            100
        );


    HAL_GPIO_WritePin(
        KX126_CS_PORT,
        KX126_CS_PIN,
        GPIO_PIN_SET
    );


    if(status != HAL_OK)
        return false;


    for(uint16_t i=0;i<length;i++)
    {
        data[i]=rx[i+1];
    }


    return true;
}
