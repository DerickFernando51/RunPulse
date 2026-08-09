#include "tasks.h"
#include "cmsis_os.h"

#include "ble.h"
#include "app_ble.h"
#include "custom_stm.h"
#include "app_entry.h"

extern "C" void BLESeqTask(void *argument)
{
    (void)argument;

    Cadence_BLE_Data_t data;

    for (;;)
    {

        MX_APPE_Process();

        // Check for new cadence data
//        if (osMessageQueueGet(
//                cadenceBleQueue,
//                &data,
//                NULL,
//                10
//            ) == osOK)
//        {
//            uint8_t blePacket[2];
//
//            blePacket[0] = data.cadence & 0xFF;
//            blePacket[1] = (data.cadence >> 8) & 0xFF;
//
//            if (APP_BLE_Get_Server_Connection_Status()
//                == APP_BLE_CONNECTED_SERVER)
//            {
//                Custom_STM_App_Update_Char(
//                    CUSTOM_STM_HR,
//                    blePacket
//                );
//            }
//        }


        // BLE INCREMENT TEST
        {
            static uint16_t testCounter = 0;

            uint8_t blePacket[2];


            testCounter++;

            blePacket[0] = testCounter & 0xFF;
            blePacket[1] = (testCounter >> 8) & 0xFF;

            if (APP_BLE_Get_Server_Connection_Status()
                == APP_BLE_CONNECTED_SERVER)
            {
                Custom_STM_App_Update_Char(
                    CUSTOM_STM_HR,
                    blePacket
                );
            }
        }



        osDelay(1);
    }
}
