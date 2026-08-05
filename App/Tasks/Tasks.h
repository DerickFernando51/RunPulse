#ifndef TASKS_H
#define TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

void SensorTask(void *argument);
void BLETask(void *argument);
void FSMTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif
