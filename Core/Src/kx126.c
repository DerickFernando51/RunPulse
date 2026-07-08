#include "kx126.h"


extern SPI_HandleTypeDef hspi1;



static void CS_LOW(void)
{
    HAL_GPIO_WritePin(KX126_CS_PORT,
                      KX126_CS_PIN,
                      GPIO_PIN_RESET);
}



static void CS_HIGH(void)
{
    HAL_GPIO_WritePin(KX126_CS_PORT,
                      KX126_CS_PIN,
                      GPIO_PIN_SET);
}




void KX126_WriteReg(uint8_t reg,uint8_t data)
{

    uint8_t tx[2];


    tx[0]=reg & 0x7F;
    tx[1]=data;


    CS_LOW();

    HAL_SPI_Transmit(&hspi1,
                     tx,
                     2,
                     100);

    CS_HIGH();

}




uint8_t KX126_ReadReg(uint8_t reg)
{

    uint8_t tx[2];
    uint8_t rx[2];


    tx[0]=reg | KX126_READ;
    tx[1]=0;


    CS_LOW();


    HAL_SPI_TransmitReceive(&hspi1,
                            tx,
                            rx,
                            2,
                            100);


    CS_HIGH();


    return rx[1];

}




void KX126_Init(void)
{

    // standby
    KX126_WriteReg(KX126_CNTL1,0x00);

    HAL_Delay(10);


    // 50Hz ODR
    KX126_WriteReg(KX126_ODCNTL,0x02);


    // enable sensor
    // PC1=1
    // high resolution
    // +-2g
    KX126_WriteReg(KX126_CNTL1,0xC0);


    HAL_Delay(20);

}





void KX126_ReadAccel(int16_t *x,
                     int16_t *y,
                     int16_t *z)
{

    uint8_t tx[7]={0};
    uint8_t rx[7]={0};


    /*
       Burst read starts at XOUT_L
    */


    tx[0]=KX126_XOUT_L | KX126_READ;



    CS_LOW();


    HAL_SPI_TransmitReceive(&hspi1,
                            tx,
                            rx,
                            7,
                            100);


    CS_HIGH();



    *x = (int16_t)((rx[2]<<8)|rx[1]);

    *y = (int16_t)((rx[4]<<8)|rx[3]);

    *z = (int16_t)((rx[6]<<8)|rx[5]);


}
