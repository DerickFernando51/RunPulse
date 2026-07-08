#ifndef KX126_H
#define KX126_H

#include "main.h"


#define KX126_CS_PORT GPIOA
#define KX126_CS_PIN  GPIO_PIN_4


#define KX126_READ 0x80


#define KX126_WHO_AM_I 0x11

#define KX126_CNTL1    0x1A
#define KX126_ODCNTL   0x1F


#define KX126_XOUT_L 0x08
#define KX126_XOUT_H 0x09

#define KX126_YOUT_L 0x0A
#define KX126_YOUT_H 0x0B

#define KX126_ZOUT_L 0x0C
#define KX126_ZOUT_H 0x0D



void KX126_Init(void);

uint8_t KX126_ReadReg(uint8_t reg);

void KX126_WriteReg(uint8_t reg,uint8_t data);

void KX126_ReadAccel(int16_t *x,
                     int16_t *y,
                     int16_t *z);


#endif
