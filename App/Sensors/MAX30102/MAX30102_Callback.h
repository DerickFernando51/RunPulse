#ifndef MAX30102_CALLBACK_H
#define MAX30102_CALLBACK_H


#ifdef __cplusplus
extern "C" {
#endif

void MAX30102_RegisterInstance(void* sensor);

void MAX30102_I2C_Callback(void* hi2c);


#ifdef __cplusplus
}
#endif


#endif
