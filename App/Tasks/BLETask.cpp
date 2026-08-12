#include "tasks.h"
#include "cmsis_os.h"

#include "ble.h"
#include "app_ble.h"
#include "custom_stm.h"
#include "app_entry.h"

extern "C" void BLETask(void *argument)
{
    (void)argument;

    BLE_Data_t data;

    for (;;)
    {
        MX_APPE_Process();

        if (osMessageQueueGet(
                bleQueue,
                &data,
                NULL,
                10
            ) == osOK)
        {
            if (APP_BLE_Get_Server_Connection_Status()
                == APP_BLE_CONNECTED_SERVER)
            {
            	uint8_t blePacket[7];

            	// Cadence [0-1]
            	blePacket[0] = data.cadence & 0xFF;
            	blePacket[1] = (data.cadence >> 8) & 0xFF;

            	// Heart rate [2-3]
            	blePacket[2] = data.heartRate & 0xFF;
            	blePacket[3] = (data.heartRate >> 8) & 0xFF;

            	// SpO2 [4]
            	blePacket[4] = data.spo2;

            	// Reserved [5]
            	blePacket[5] = 0;

            	// Battery SOC [6]
            	blePacket[6] = data.batterySOC;

                Custom_STM_App_Update_Char(
                    CUSTOM_STM_HR,
                    blePacket
                );
            }
        }

        osDelay(1);
    }
}
